#include "explorer.hpp"
#include <limits>

namespace fmo {
    namespace {
        constexpr int int16_max = std::numeric_limits<int16_t>::max();

        struct Init {
            Init() {
                Algorithm::registerFactory("explorer-v1", [](const Algorithm::Config& config) {
                    return std::unique_ptr<Algorithm>(new ExplorerV1(config));
                });
            }
        } init;
    }

    ExplorerV1::~ExplorerV1() = default;

    ExplorerV1::ExplorerV1(const Config& cfg) : mCfg(cfg) {
        if (mCfg.dims.width <= 0 || mCfg.dims.height <= 0 || mCfg.dims.width > int16_max ||
            mCfg.dims.height > int16_max) {
            throw std::runtime_error("bad config");
        }

        if (mCfg.dims.height <= mCfg.maxHeight) {
            throw std::runtime_error("bad config: expecting height to be larger than maxHeight");
        }

        // allocate the source level
        int step = 1;
        int width = mCfg.dims.width;
        int height = mCfg.dims.height;

        mSourceLevel.image1.resize(Format::GRAY, {width, height});
        mSourceLevel.image2.resize(Format::GRAY, {width, height});
        mSourceLevel.image3.resize(Format::GRAY, {width, height});

        step *= 2;
        width /= 2;
        height /= 2;

        // create as many decimation levels as required to get below maximum height
        mIgnoredLevels.reserve(4);
        while (height > mCfg.maxHeight) {
            mIgnoredLevels.emplace_back();
            mIgnoredLevels.back().image.resize(Format::GRAY, {width, height});

            step *= 2;
            width /= 2;
            height /= 2;
        }

        // allocate the processed level
        mLevel.image1.resize(Format::GRAY, {width, height});
        mLevel.image2.resize(Format::GRAY, {width, height});
        mLevel.image3.resize(Format::GRAY, {width, height});
        mLevel.diff1.resize(Format::GRAY, {width, height});
        mLevel.diff2.resize(Format::GRAY, {width, height});
        mLevel.preprocessed.resize(Format::GRAY, {width, height});
        mLevel.step = step;
    }

    void ExplorerV1::setInputSwap(Image& input) {
        mFrameNum++;
        createLevelPyramid(input);
        preprocess();
        findStrips();
        findComponents();
        analyzeComponents();
        findTrajectories();
        analyzeTrajectories();
        findObjects();
    }
}
