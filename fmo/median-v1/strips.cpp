#include "algorithm-median.hpp"

namespace fmo {
    void MedianV1::findStrips() {
        auto& input = mProcessingLevel.binDiff;
        int minHeight = mCfg.minStripHeight;
        int minGap = int(mCfg.minGap * input.dims().height);
        int step = 1 << mProcessingLevel.pixelSizeLog2;
        int outNoise;
        mStripGen(input, minHeight, minGap, step, mStrips, outNoise);
        mDiff.reportAmountOfNoise(outNoise);

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
