#include "algorithm-median.hpp"

namespace fmo {
    void MedianV1::findStrips() {
        auto& input = mProcessingLevel.binDiff;
        int minHeight = mCfg.minStripHeight;
        int minGap = int(mCfg.minGap * input.dims().height);
        int step = 1 << mProcessingLevel.pixelSizeLog2;
        int outNoise;
        mStripGen(input, minHeight, minGap, step, mStrips, outNoise);

        // // evaluate the amount of noise, adjust the threshold accordingly
        // bool updated = mCache.noiseStats.add(outNoise);
        // if (updated) {
        //     int numPixels = input.dims().width * input.dims().height;
        //     double noiseFrac = double(mCache.noiseStats.quantiles().q50) / numPixels;
        //     if (noiseFrac > 0.00250) mDiff.requestLessSensitive();
        //     if (noiseFrac < 0.00125) mDiff.requestMoreSensitive();
        // }

        // sort strips by x coordinate
        std::sort(begin(mStrips), end(mStrips), [](const Strip& l, const Strip& r) {
            if (l.pos.x == r.pos.x) {
                return l.pos.y < r.pos.y;
            } else {
                return l.pos.x < r.pos.x;
            }
        });
    }
}
