#include "explorer.hpp"
#include <fmo/stripgen-impl.hpp>

namespace fmo {
    void ExplorerV3::findProtoStrips() {
        if (mFrameNum < 2) return;
        mLevel.strips1.swap(mLevel.strips2);
        mLevel.strips1.clear();

        Dims dims = mLevel.diff1.dims();
        int minHeight = mCfg.minStripHeight;
        int minGap = int(mCfg.minGap * dims.height);
        int step = mLevel.step;
        auto& out = reinterpret_cast<std::vector<StripRepr>&>(mLevel.strips1);
        int outNoise = 0;
        mStripGen(mLevel.diff1, minHeight, minGap, step, out, outNoise);

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
