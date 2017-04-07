#include "explorer.hpp"

namespace fmo {
    void ExplorerV2::findStrips() {
        if (mFrameNum >= 3) {
            mStrips.clear();
            findStrips(mLevel);
        }
    }

    void ExplorerV2::findStrips(ProcessedLevel& level) {
        Dims dims = level.preprocessed.dims();
        int minHeight = mCfg.minStripHeight;
        int minGap = int(mCfg.minGap * dims.height);
        int step = level.step;
        int outNoise;
        mStripGen(level.preprocessed, minHeight, minGap, step, mStrips, outNoise);

        // set next strip in component to a special value
        for (auto& strip : mStrips) { next(strip) = Special::UNTOUCHED; }

        // evaluate the amount of noise, adjust the threshold accordingly
        bool updated = mCache.noiseStats.add(outNoise);
        if (updated) {
            double noiseFrac =
                double(mCache.noiseStats.quantiles().q50) / (dims.width * dims.height);
            if (noiseFrac > 0.00250) mNoiseAdjust += 1;
            if (noiseFrac < 0.00125) mNoiseAdjust -= 1;
        }
    }
}
