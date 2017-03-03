#include "explorer-impl.hpp"
#include <limits>

namespace fmo {
    namespace {
        constexpr int int16_max = std::numeric_limits<int16_t>::max();
    }

    Explorer::~Explorer() = default;
    Explorer::Explorer(const Config& cfg) : mImpl(new Impl(cfg)) {}
    Explorer::Explorer(Explorer&&) = default;
    Explorer& Explorer::operator=(Explorer&&) = default;
    void Explorer::setInput(const Mat& src) { mImpl->setInput(src); }
    const Image& Explorer::getDebugImage() { return mImpl->getDebugImage(); }

    inline Explorer::Impl::Impl(const Config& cfg) : mCfg(cfg) {
        if (mCfg.dims.width <= 0 || mCfg.dims.height <= 0 || mCfg.dims.width > int16_max ||
            mCfg.dims.height > int16_max) {
            throw std::runtime_error("bad config");
        }

        // create as many decimation levels as required to reach minimum height
        int step = 2;
        int width = mCfg.dims.width / 2;
        mIgnoredLevels.reserve(4);
        mLevels.reserve(MAX_LEVELS);
        for (int height = mCfg.dims.height / 2; height >= mCfg.minHeight; height /= 2) {
            if (height > mCfg.maxHeight) {
                mIgnoredLevels.emplace_back();
                mIgnoredLevels.back().image.resize(Format::GRAY, {width, height});
            } else {
                mLevels.emplace_back();
                mLevels.back().image1.resize(Format::GRAY, {width, height});
                mLevels.back().image2.resize(Format::GRAY, {width, height});
                mLevels.back().image3.resize(Format::GRAY, {width, height});
                mLevels.back().diff1.resize(Format::GRAY, {width, height});
                mLevels.back().diff2.resize(Format::GRAY, {width, height});
                mLevels.back().preprocessed.resize(Format::GRAY, {width, height});
                mLevels.back().step = step;
                if (mLevels.size() == MAX_LEVELS) break;
            }
            step *= 2;
            width /= 2;
        }
    }

    inline void Explorer::Impl::setInput(const Mat& src) {
        mFrameNum++;
        createLevelPyramid(src);
        preprocess();
        findStrips();
        findComponents();
        analyzeComponents();
        findTrajectories();
        analyzeTrajectories();
    }
}
