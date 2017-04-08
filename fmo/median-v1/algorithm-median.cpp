#include "algorithm-median.hpp"
#include "../include-opencv.hpp"
#include <fmo/processing.hpp>

namespace fmo {
    void registerMedianV1() {
        Algorithm::registerFactory(
            "median-v1", [](const Algorithm::Config& config, Format format, Dims dims) {
                return std::unique_ptr<Algorithm>(new MedianV1(config, format, dims));
            });
    }

    MedianV1::MedianV1(const Config& cfg, Format format, Dims dims)
        : mCfg(cfg), mSourceLevel{{format, dims}, 0} {}

    void MedianV1::setInputSwap(Image& in) {
        swapAndDecimateInput(in);
        computeBinDiff();
        findStrips();
        // add steps here...
    }

    void MedianV1::swapAndDecimateInput(Image& in) {
        if (in.format() != mSourceLevel.image.format()) {
            throw std::runtime_error("setInputSwap(): bad format");
        }

        if (in.dims() != mSourceLevel.image.dims()) {
            throw std::runtime_error("setInputSwap(): bad dimensions");
        }

        mSourceLevel.image.swap(in);
        mSourceLevel.frameNum++;

        // decimate until the image size is below a set height
        int pixelSizeLog2 = 0;
        Image* input = &mSourceLevel.image;

        for (; input->dims().height > mCfg.maxImageHeight; pixelSizeLog2++) {
            if (int(mCache.decimated.size()) == pixelSizeLog2) {
                mCache.decimated.emplace_back(new Image);
            }
            auto& next = *mCache.decimated[pixelSizeLog2];
            mDecimator(*input, next);
            input = &next;
        }

        // need at least one decimation to happen
        // - because strips use integral half heights
        // - becuase we want the source image untouched
        if (pixelSizeLog2 == 0) {
            throw std::runtime_error("setInputSwap(): input image too small");
        }

        // swap the product of decimation into the processing level
        mProcessingLevel.inputs[2].swap(mProcessingLevel.inputs[1]);
        mProcessingLevel.inputs[1].swap(mProcessingLevel.inputs[0]);
        mProcessingLevel.inputs[0].swap(*input);
        mProcessingLevel.pixelSizeLog2 = pixelSizeLog2;
    }

    void MedianV1::computeBinDiff() {
        auto& level = mProcessingLevel;

        if (mSourceLevel.frameNum < 3) {
            // initial frames: just generate a black diff
            level.binDiff.resize(Format::GRAY, level.inputs[0].dims());
            level.binDiff.wrap().setTo(uint8_t(0x00));
            return;
        }

        fmo::median3(level.inputs[0], level.inputs[1], level.inputs[2], level.background);
        mDiff(mCfg.diff, level.inputs[0], level.background, level.binDiff, 0);
    }

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
