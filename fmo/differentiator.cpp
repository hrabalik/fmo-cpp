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
            int numPixels = mDiff.dims().width * mDiff.dims().height;

            cv::Mat diffMat = mDiff.wrap();
            diffMat.reshape(1, numPixels);

            mSum.resize(Format::GRAY, mDiff.dims());
            cv::Mat sumMat = mSum.wrap();
            sumMat.reshape(1, numPixels);

            cv::reduce(diffMat, sumMat, 1, cv::REDUCE_SUM, CV_8UC1);
            greater_than(mSum, dst, bgr ? config.threshBgr : config.threshYuv);
            return;
        }
        default:
            throw std::runtime_error("Differentiator: unsupported format");
        }
    }
}
