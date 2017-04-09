#include "../include-opencv.hpp"
#include "algorithm-median.hpp"
#include <fmo/processing.hpp>

namespace fmo {
    namespace {
        const cv::Scalar inactiveStripsColor{0x20, 0x20, 0x20};
    }

    const Image& MedianV1::getDebugImage() {
        // convert to BGR
        fmo::copy(mProcessingLevel.binDiff, mCache.diffConverted, Format::BGR);
        fmo::copy(mProcessingLevel.inputs[0], mCache.inputConverted, Format::BGR);

        // scale to source size
        cv::Mat cvDiff;
        cv::Mat cvVis;
        {
            mCache.diffScaled.resize(Format::BGR, mSourceLevel.image.dims());
            mCache.visualized.resize(Format::BGR, mSourceLevel.image.dims());
            cvDiff = mCache.diffScaled.wrap();
            cvVis = mCache.visualized.wrap();
            cv::resize(mCache.diffConverted.wrap(), cvDiff, cvDiff.size(), 0, 0, cv::INTER_NEAREST);
            cv::resize(mCache.inputConverted.wrap(), cvVis, cvVis.size(), 0, 0, cv::INTER_NEAREST);
        }

        // blend diff and input
        cv::addWeighted(cvDiff, 0.5, cvVis, 0.5, 0, cvVis);

        // draw strips
        for (auto& strip : mStrips) {
            cv::Point p1{strip.pos.x - strip.halfDims.width, strip.pos.y - strip.halfDims.height};
            cv::Point p2{strip.pos.x + strip.halfDims.width, strip.pos.y + strip.halfDims.height};
            cv::rectangle(cvVis, p1, p2, inactiveStripsColor);
        }

        return mCache.visualized;
    }
}
