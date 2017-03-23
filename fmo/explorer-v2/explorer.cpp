#include "explorer.hpp"
#include <limits>

namespace fmo {
    namespace {
        constexpr int int16_max = std::numeric_limits<int16_t>::max();
    }

    void registerExplorerV2() {
        Algorithm::registerFactory("explorer-v2", [] (const Algorithm::Config& config) {
            return std::unique_ptr<Algorithm>(new ExplorerV2(config));
        });
    }

    ExplorerV2::~ExplorerV2() = default;

    ExplorerV2::ExplorerV2(const Config& cfg) : mCfg(cfg) {
        if (mCfg.dims.width <= 0 || mCfg.dims.height <= 0 || mCfg.dims.width > int16_max ||
            mCfg.dims.height > int16_max) {
            throw std::runtime_error("bad config");
        }

        if (mCfg.dims.height <= mCfg.maxImageHeight) {
            throw std::runtime_error("bad config: expecting height to be larger than maxImageHeight");
        }

        if (mCfg.format != Format::GRAY && mCfg.format != Format::BGR) {
            throw std::runtime_error("bad format");
        }

        // allocate the source level
        int step = 1;
        Dims dims = mCfg.dims;
        Format format = mCfg.format;
        //int width = mCfg.dims.width;
        //int height = mCfg.dims.height;

        mSourceLevel.image1.resize(format, dims);
        mSourceLevel.image2.resize(format, dims);
        mSourceLevel.image3.resize(format, dims);

        format = mDecimator.nextFormat(format);
        dims = mDecimator.nextDims(dims);
        step = mDecimator.nextPixelSize(step);

        // create as many decimation levels as required to get below maximum height
        mIgnoredLevels.reserve(4);
        while (dims.height > mCfg.maxImageHeight) {
            mIgnoredLevels.emplace_back();
            mIgnoredLevels.back().image.resize(format, dims);

            format = mDecimator.nextFormat(format);
            dims = mDecimator.nextDims(dims);
            step = mDecimator.nextPixelSize(step);
        }

        // allocate the processed level
        mLevel.image1.resize(format, dims);
        mLevel.image2.resize(format, dims);
        mLevel.image3.resize(format, dims);
        mLevel.diff1.resize(Format::GRAY, dims);
        mLevel.diff2.resize(Format::GRAY, dims);
        mLevel.preprocessed.resize(Format::GRAY, dims);
        mLevel.step = step;
    }

    void ExplorerV2::setInputSwap(Image& input) {
        mFrameNum++;
        createLevelPyramid(input);
        preprocess();
        findStrips();
        findComponents();
        findClusters();
        // analyzeComponents();
        // findTrajectories();
        // analyzeTrajectories();
        // findObjects();
    }
}
