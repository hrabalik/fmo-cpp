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
        : mCfg(cfg), mSourceLevel{{format, dims}, 0}, mDiff(cfg.diff) {}

    void MedianV1::setInputSwap(Image& in) {
        swapAndSubsampleInput(in);
        computeBinDiff();
        findComponents();
        findObjects();
        matchObjects();
        selectObjects();
        // add steps here...
    }

    void MedianV1::swapAndSubsampleInput(Image& in) {
        if (in.format() != mSourceLevel.image.format()) {
            throw std::runtime_error("setInputSwap(): bad format");
        }

        if (in.dims() != mSourceLevel.image.dims()) {
            throw std::runtime_error("setInputSwap(): bad dimensions");
        }

        mSourceLevel.image.swap(in);
        mSourceLevel.frameNum++;

        // subsample until the image size is below a set height
        int pixelSizeLog2 = 0;
        Image* input = &mSourceLevel.image;

        for (; input->dims().height > mCfg.maxImageHeight; pixelSizeLog2++) {
            if (int(mCache.subsampled.size()) == pixelSizeLog2) {
                mCache.subsampled.emplace_back(new Image);
            }
            auto& next = *mCache.subsampled[pixelSizeLog2];
            mSubsampler(*input, next);
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
        mDiff(level.inputs[0], level.background, level.binDiff);
    }
}
