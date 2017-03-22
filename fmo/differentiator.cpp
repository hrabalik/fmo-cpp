#include "include-opencv.hpp"
#include <fmo/assert.hpp>
#include <fmo/differentiator.hpp>
#include <fmo/processing.hpp>

namespace fmo {
    Differentiator::Config::Config() : threshGray(19), threshBgr(19), threshYuv(19) {}

    void addAndThresh(const Image& src, Mat& dst, int thresh) {
        Dims dims = src.dims();
        Format format = src.format();
        if (format != Format::BGR && format != Format::YUV) {
            throw std::runtime_error("addChannels(): bad format");
        }
        dst.resize(Format::GRAY, dims);

        auto* out = dst.data();
        auto* data = src.data();
        auto* end = data + (dims.width * dims.height * 3);
        for (; data < end; data += 3, out++) {
            *out = ((data[0] + data[1] + data[2]) > thresh) ? uint8_t(0xFF) : uint8_t(0);
        }
    }

    void Differentiator::operator()(const Config& config, const Mat& src1, const Mat& src2,
                                    Mat& dst) {
        absdiff(src1, src2, mDiff);
        Format format = mDiff.format();

        switch (format) {
        case Format::GRAY: {
            greater_than(mDiff, dst, config.threshGray);
            return;
        }
        case Format::BGR:
        case Format::YUV: {
            bool bgr = format == Format::BGR;
            int thresh = bgr ? config.threshBgr : config.threshYuv;
            addAndThresh(mDiff, dst, thresh);
            return;
        }
        default:
            throw std::runtime_error("Differentiator: unsupported format");
        }
    }
}
