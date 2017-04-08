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
        computeBackground();
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

    void MedianV1::computeBackground() {
        auto& level = mProcessingLevel;

        if (mSourceLevel.frameNum < 3) {
            // initial frames: just use the latest image as background
            fmo::copy(level.inputs[0], level.background);
        } else {
            fmo::median3(level.inputs[0], level.inputs[1], level.inputs[2], level.background);
        }
    }

    const Image& MedianV1::getDebugImage() {
        // convert background to BGR
        fmo::copy(mProcessingLevel.background, mCache.converted, Format::BGR);

        // scale background to source size
        cv::Mat cvVis;
        {
            mCache.visualized.resize(Format::BGR, mSourceLevel.image.dims());
            cvVis = mCache.visualized.wrap();
            cv::resize(mCache.converted.wrap(), cvVis, cvVis.size(), 0, 0, cv::INTER_NEAREST);
        }

        return mCache.visualized;
    }
}
