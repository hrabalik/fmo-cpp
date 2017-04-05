#include "explorer.hpp"
#include <fmo/stripgen-impl.hpp>

namespace fmo {
    void ExplorerV3::findProtoStrips() {
        if (mFrameNum < 2) return;
        mLevel.strips1.swap(mLevel.strips2);
        mLevel.strips1.clear();

        auto addFunc = [this](const Pos16& pos, const Dims16& halfDims) {
            mLevel.strips1.emplace_back(pos, halfDims);
        };

        Dims dims = mLevel.diff1.dims();
        int minHeight = mCfg.minStripHeight;
        int minGap = int(mCfg.minGap * dims.height);
        int step = mLevel.step;
        mStripGen(mLevel.diff1, minHeight, minGap, step, addFunc);

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
