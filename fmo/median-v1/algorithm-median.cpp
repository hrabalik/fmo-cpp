#include "algorithm-median.hpp"

namespace fmo {
    void registerMedianV1() {
        Algorithm::registerFactory(
            "median-v1", [](const Algorithm::Config& config, Format format, Dims dims) {
                return std::unique_ptr<Algorithm>(new MedianV1(config, format, dims));
            });
    }

    MedianV1::MedianV1(const Config& cfg, Format format, Dims dims)
        : mCfg(cfg), mSourceLevel{{format, dims}} {}

    void MedianV1::setInputSwap(Image& in) {
        swapAndDecimateInput(in);
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

        // decimate until the image size is below a set height
        int pixelSizeLog2 = 0;
        Image* input = &mSourceLevel.image;

        for (; input->dims().height > mCfg.maxImageHeight; pixelSizeLog2++) {
            if (int(mCache.decimated.size()) == pixelSizeLog2) { mCache.decimated.emplace_back(); }
            auto& next = mCache.decimated[pixelSizeLog2 - 1];
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
}
