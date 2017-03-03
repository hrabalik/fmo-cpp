#include "explorer-impl.hpp"

namespace fmo {
    void Explorer::Impl::findStrips() {
        mStrips.clear();
        for (auto& level : mLevels) {
            findStrips(level);
        }
    }

    void Explorer::Impl::findStrips(Level& level) {
        if (mFrameNum < 3) return;
        level.numStrips = 0;
        Dims dims = level.preprocessed.dims();
        uint8_t* colData = level.preprocessed.data();

        int blackPrev = 0;
        int black = 0;
        int whitePrev = 0;
        int white = 0;
        int col = 0;
        int row = 0;
        int step = level.step;
        int halfStep = level.step / 2;
        int minGap = int(mCfg.minGap * dims.height);

        // Called after a black strip has ended (including at the end of the column). Creates a
        // strip if the previous two black and one white strip satisfy all conditions.
        auto check = [&, this]() {
            if (blackPrev >= minGap && black >= minGap && whitePrev > 0 && whitePrev >= 1) {
                int halfHeight = whitePrev * halfStep;
                int x = (col * step) + halfStep;
                int y = ((row - black) * step) - halfHeight;
                mStrips.emplace_back(int16_t(x), int16_t(y), int16_t(halfHeight));
                level.numStrips++;
            }
        };

        for (col = 0; col < dims.width; col++, colData++) {
            uint8_t* data = colData;
            blackPrev = 0;
            black = minGap; // don't limit by top edge
            whitePrev = 0;
            white = 0;

            for (row = 0; row < dims.height; row++, data += dims.width) {
                if (*data != 0) {
                    if (white++ == 0) {
                        check();
                        blackPrev = black;
                        black = 0;
                    }
                } else {
                    if (black++ == 0) {
                        whitePrev = white;
                        white = 0;
                    }
                }
            }
            black += minGap; // don't limit by bottom edge
            row += minGap;   // don't limit by bottom edge
            check();
        }
    }
}
