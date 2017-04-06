#include "image-util.hpp"
#include "include-opencv.hpp"
#include "include-simd.hpp"
#include <array>
#include <fmo/assert.hpp>
#include <fmo/differentiator.hpp>
#include <fmo/processing.hpp>

namespace fmo {
    Differentiator::Config::Config() : threshGray(19), threshBgr(23), threshYuv(23) {}

    struct AddAndThreshJob : public cv::ParallelLoopBody {
#if defined(FMO_HAVE_SSE2)
        using batch_t = __m128i;

        static void impl(const uint8_t* src, const uint8_t* srcEnd, uint8_t* dst, uint8_t thresh) {
            alignas(batch_t) std::array<uint8_t, sizeof(batch_t)> x;
            alignas(batch_t) std::array<uint8_t, sizeof(batch_t)> y;
            alignas(batch_t) std::array<uint8_t, sizeof(batch_t)> z;

            x.fill(0x80);
            batch_t xorVec = _mm_load_si128((const batch_t*) x.data());
            x.fill(uint8_t(thresh));
            batch_t threshVec = _mm_load_si128((const batch_t*)x.data());
            threshVec = _mm_xor_si128(threshVec, xorVec);

            for (; src < srcEnd; dst += sizeof(batch_t)) {
                for (int i = 0; i < 16; i++) {
                    x[i] = *(src++);
                    y[i] = *(src++);
                    z[i] = *(src++);
                }
                batch_t xVec = _mm_load_si128((const batch_t*)x.data());
                batch_t yVec = _mm_load_si128((const batch_t*)y.data());
                batch_t zVec = _mm_load_si128((const batch_t*)z.data());
                xVec = _mm_adds_epu8(xVec, yVec);
                xVec = _mm_adds_epu8(xVec, zVec);
                xVec = _mm_xor_si128(xVec, xorVec);
                xVec = _mm_cmpgt_epi8(xVec, threshVec);
                _mm_stream_si128((batch_t*)dst, xVec);
            }
        }
#else
        using batch_t = uint8_t;

        static void impl(const uint8_t* src, const uint8_t* srcEnd, uint8_t* dst, uint8_t thresh) {
            for (; src < srcEnd; src += SRC_BATCH_SIZE, dst += DST_BATCH_SIZE) {
                *dst = ((src[0] + src[1] + src[2]) > thresh) ? uint8_t(0xFF) : uint8_t(0);
            }
        }
#endif

        enum : size_t {
            SRC_BATCH_SIZE = sizeof(batch_t) * 3,
            DST_BATCH_SIZE = sizeof(batch_t),
        };

        AddAndThreshJob(const uint8_t* src, uint8_t* dst, int thresh)
            : mSrc(src), mDst(dst), mThresh(uint8_t(thresh)) {}

        virtual void operator()(const cv::Range& pieces) const override {
            size_t firstSrc = size_t(pieces.start) * SRC_BATCH_SIZE;
            size_t lastSrc = size_t(pieces.end) * SRC_BATCH_SIZE;
            size_t firstDst = size_t(pieces.start) * DST_BATCH_SIZE;
            const uint8_t* src = mSrc + firstSrc;
            const uint8_t* srcEnd = mSrc + lastSrc;
            uint8_t* dst = mDst + firstDst;
            impl(src, srcEnd, dst, mThresh);
        }

    private:
        const uint8_t* const mSrc;
        uint8_t* const mDst;
        const uint8_t mThresh;
    };

    void addAndThresh(const Image& src, Image& dst, int thresh) {
        const Format format = src.format();
        const Dims dims = src.dims();
        const size_t pixels = size_t(dims.width) * size_t(dims.height);
        const size_t pieces = pixels / AddAndThreshJob::DST_BATCH_SIZE;

        if (getPixelStep(format) != 3) { throw std::runtime_error("addAndThresh(): bad format"); }

        // run the job in parallel
        dst.resize(Format::GRAY, dims);
        AddAndThreshJob job{src.data(), dst.data(), thresh};
        cv::parallel_for_(cv::Range{0, int(pieces)}, job, cv::getNumThreads());

        // process the last few bytes individually
        size_t lastIndex = pieces * AddAndThreshJob::DST_BATCH_SIZE;
        const uint8_t* data = src.data() + (lastIndex * 3);
        uint8_t* out = dst.data() + lastIndex;
        uint8_t* outEnd = dst.data() + pixels;
        for (; out < outEnd; out++, data += 3) {
            *out = ((data[0] + data[1] + data[2]) > thresh) ? uint8_t(0xFF) : uint8_t(0);
        }
    }

    void Differentiator::operator()(const Config& config, const Mat& src1, const Mat& src2,
                                    Image& dst, int adjust) {
        absdiff(src1, src2, mDiff);
        Format format = mDiff.format();

        switch (format) {
        case Format::GRAY: {
            uint8_t adjusted = uint8_t(int(config.threshGray) + adjust);
            greater_than(mDiff, dst, adjusted);
            return;
        }
        case Format::BGR:
        case Format::YUV: {
            bool bgr = format == Format::BGR;
            int thresh = bgr ? config.threshBgr : config.threshYuv;
            addAndThresh(mDiff, dst, thresh + adjust);
            return;
        }
        default:
            throw std::runtime_error("Differentiator: unsupported format");
        }
    }
}
