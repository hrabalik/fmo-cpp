#include "include-opencv.hpp"
#include <fmo/assert.hpp>
#include <fmo/differentiator.hpp>
#include <fmo/processing.hpp>

namespace fmo {
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
            chan1.resize(Format::GRAY, mDiff.dims());
            chan2.resize(Format::GRAY, mDiff.dims());
            chan3.resize(Format::GRAY, mDiff.dims());
            mSum.resize(Format::GRAY, mDiff.dims());
            dst.resize(Format::GRAY, mDiff.dims());
            cv::Mat mats[3] = {chan1.wrap(), chan2.wrap(), chan3.wrap()};
            cv::split(mDiff.wrap(), mats);
            cv::addWeighted(mats[0], 1, mats[1], 1, 0, mats[1]);
            cv::addWeighted(mats[1], 1, mats[2], 1, 0, mSum.wrap());
            greater_than(mSum, dst, bgr ? config.threshBgr : config.threshYuv);
            return;
        }
        default:
            throw std::runtime_error("Differentiator: unsupported format");
        }
    }
}
