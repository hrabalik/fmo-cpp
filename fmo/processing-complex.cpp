#include "image-util.hpp"
#include <fmo/processing.hpp>

namespace fmo {
    void decimate(const Mat& src, Mat& dst) {
        if (src.format() == Format::YUV420SP) {
            throw std::runtime_error("downscale: source cannot be YUV420SP");
        }

        Dims srcDims = src.dims();
        Dims dstDims = {srcDims.width / 2, srcDims.height / 2};

        if (dstDims.width == 0 || dstDims.height == 0) {
            throw std::runtime_error("downscale: source is too small");
        }

        dst.resize(src.format(), dstDims);
        cv::Mat srcMat = src.wrap();
        cv::Mat dstMat = dst.wrap();

        if (srcDims.width % 2 != 0) { srcMat.flags &= ~cv::Mat::CONTINUOUS_FLAG; }

        srcMat.cols &= ~1;
        srcMat.rows &= ~1;

        cv::resize(srcMat, dstMat, cv::Size(dstDims.width, dstDims.height), 0, 0, cv::INTER_AREA);
    }

    struct Median3Job : public cv::ParallelLoopBody {
        using batch_t = uint8_t;

        enum : size_t {
            BATCH = sizeof(batch_t),
        };

        Median3Job(const Mat& src1, const Mat& src2, const Mat& src3, Mat& dst)
            : mSrc1(src1.data()),
              mSrc2(src2.data()),
              mSrc3(src3.data()),
              mDst(dst.data())
              //mAligned(size_t(mSrc1) % BATCH == 0 && size_t(mSrc2) % BATCH == 0 &&
              //         size_t(mSrc3) % BATCH == 0 && size_t(mDst) % BATCH == 0)
        {}

        virtual void operator()(const cv::Range& pieces) const override {
            size_t first = size_t(pieces.start) * BATCH;
            size_t last = size_t(pieces.end) * BATCH;
            fallbackImpl(first, last);
        }

        void fallbackImpl(size_t first, size_t last) const {
            const uint8_t* const src1 = mSrc1 + first;
            const uint8_t* const src2 = mSrc2 + first;
            const uint8_t* const src3 = mSrc3 + first;
            uint8_t* const dst = mDst + first;
            const size_t iEnd = last - first;

            for (size_t i = 0; i < iEnd; i++) {
                uint8_t t = std::max(src1[i], src2[i]);
                uint8_t s = std::min(src1[i], src2[i]);
                t = std::min(t, src3[i]);
                dst[i] = std::max(s, t);
            }
        }

    private:
        const uint8_t* const mSrc1;
        const uint8_t* const mSrc2;
        const uint8_t* const mSrc3;
        uint8_t* const mDst;
        //const bool mAligned;
    };

    void median3(const Image& src1, const Image& src2, const Image& src3, Image& dst) {
        const Format format = src1.format();
        const Dims dims = src1.dims();
        const cv::Size size = getCvSize(format, dims);
        const size_t bytes = size_t(size.width) * size_t(size.height) * getPixelStep(format);
        const size_t pieces = bytes / Median3Job::BATCH;

        if (format != src2.format() || dims != src2.dims() || format != src3.format() ||
            dims != src3.dims()) {
            throw std::runtime_error("median3: format/dimensions mismatch of inputs");
        }

        // run the job in parallel
        dst.resize(format, dims);
        Median3Job job{src1, src2, src3, dst};
        cv::parallel_for_(cv::Range{0, int(pieces)}, job, cv::getNumThreads());

        // process the last few bytes inidividually
        for (size_t i = pieces * Median3Job::BATCH; i < bytes; i++) {
            uint8_t t = std::max(src1.data()[i], src2.data()[i]);
            uint8_t s = std::min(src1.data()[i], src2.data()[i]);
            t = std::min(t, src3.data()[i]);
            dst.data()[i] = std::max(s, t);
        }
    }
}
