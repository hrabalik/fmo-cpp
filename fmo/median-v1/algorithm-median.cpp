#include "algorithm-median.hpp"

namespace fmo {
    void registerMedianV1() {
        Algorithm::registerFactory(
            "median-v1", [](const Algorithm::Config& config, Format format, Dims dims) {
                return std::unique_ptr<Algorithm>(new MedianV1(config, format, dims));
            });
    }

    MedianV1::MedianV1(const Config& cfg, Format format, Dims dims)
        : mCfg(cfg), mSourceLevel{format, dims} {}
}
