#include "explorer.hpp"
#include <fmo/stripgen-impl.hpp>

namespace fmo {
    void ExplorerV2::findStrips() {
        if (mFrameNum >= 3) {
            mStrips.clear();
            findStrips(mLevel);
        }
    }

    void ExplorerV2::findStrips(ProcessedLevel& level) {
        level.numStrips = 0;

        auto addFunc = [this, &level](const Pos16& pos, const Dims16& halfDims) {
            mStrips.emplace_back(pos, halfDims.height);
            level.numStrips++;
        };

        Dims dims = level.preprocessed.dims();
        int minHeight = mCfg.minStripHeight;
        int minGap = int(mCfg.minGap * dims.height);
        int step = level.step;
        mStripGen(level.preprocessed, minHeight, minGap, step, addFunc);

        // evaluate the amount of noise, adjust the threshold accordingly
        bool updated = mCache.noiseStats.add(mStripGen.getNoise());
        if (updated) {
            double noiseFrac =
                double(mCache.noiseStats.quantiles().q50) / (dims.width * dims.height);
            if (noiseFrac > 0.00250) mNoiseAdjust += 1;
            if (noiseFrac < 0.00125) mNoiseAdjust -= 1;
        }
    }
}
