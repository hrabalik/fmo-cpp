#include "explorer.hpp"

namespace fmo {
    void ExplorerV3::findProtoStrips() {
        if (mFrameNum < 2) return;
        mLevel.strips1.swap(mLevel.strips2);
        mLevel.strips1.clear();

        const Dims dims = mLevel.diff1.dims();
        const int step = mLevel.step;
        const int halfStep = mLevel.step / 2;
        const int minGap = int(mCfg.minGap * dims.height);

        struct {
            int black2Prev = 0;
            int blackPrev = 0;
            int black = 0;
            int whitePrev = 0;
            int white = 0;
            int col = 0;
            int row = 0;
        } s;

        uint8_t* colData = mLevel.diff1.data();
        int noise = 0;

        // Called after a white strip has ended. Stores a strip if the previous two black and one
        // white strip satisfy all conditions.
        auto check = [this, &s, step, halfStep, minGap]() {
            if (s.black2Prev >= minGap && s.blackPrev >= minGap && s.whitePrev > 0) {
                int halfHeight = s.whitePrev * halfStep;
                int x = (s.col * step) + halfStep;
                int y = ((s.row - s.white - s.blackPrev) * step) - halfHeight;
                mLevel.strips1.emplace_back(Pos16{int16_t(x), int16_t(y)},
                                            Dims16{int16_t(halfStep), int16_t(halfHeight)});
            }
        };

        for (s.col = 0; s.col < dims.width; s.col++, colData++) {
            uint8_t* data = colData;
            s.black2Prev = 0;
            s.blackPrev = 0;
            s.black = minGap; // hack: don't limit by top edge
            s.whitePrev = 0;
            s.white = 0;

            for (s.row = 0; s.row < dims.height; s.row++, data += dims.width) {
                if (*data != 0) {
                    if (s.white++ == 0) {
                        s.black2Prev = s.blackPrev;
                        s.blackPrev = s.black;
                        s.black = 0;
                    }
                } else {
                    if (s.black++ == 0) {
                        if (s.white >= mCfg.minStripHeight) {
                            check();
                            s.whitePrev = s.white;
                            s.white = 0;
                        } else {
                            noise++;
                            s.black = s.blackPrev + s.white + s.black;
                            s.blackPrev = s.black2Prev;
                            s.black2Prev = 0;
                            s.white = 0;
                        }
                    }
                }
            }

            // hack: don't limit by bottom edge
            if (s.white != 0) {
                check();
            } else {
                s.black2Prev = s.blackPrev;
                s.blackPrev = s.black + minGap;
                s.white = 0;
                s.row = s.row + minGap;
                check();
            }
        }

        // evaluate the amount of noise, adjust the threshold accordingly
        bool updated = mCache.noiseStats.add(noise);
        if (updated) {
            double noiseFrac =
                double(mCache.noiseStats.quantiles().q50) / (dims.width * dims.height);
            if (noiseFrac > 0.00250) mNoiseAdjust += 1;
            if (noiseFrac < 0.00125) mNoiseAdjust -= 1;
        }
    }
}
