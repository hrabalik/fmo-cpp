#include "explorer.hpp"
#include <algorithm>
#include <fmo/algebra.hpp>
#include <fmo/assert.hpp>

namespace fmo {
    namespace {
        constexpr int int16_max = std::numeric_limits<int16_t>::max();
    }

    void ExplorerV2::findComponents() {
        // reset
        mComponents.clear();
        int step = mLevel.step;

        FMO_ASSERT(mStrips.size() < size_t(int16_max), "too many strips");
        int numStrips = int(mStrips.size());

        for (int i = 0; i < numStrips; i++) {
            StripRepr& me = mStrips[i];

            // create new components for previously untouched strips
            if (next(me) == Special::UNTOUCHED) { mComponents.emplace_back(int16_t(i)); }

            // find the next strip
            next(me) = Special::END;
            for (int j = i + 1; j < numStrips; j++) {
                StripRepr& candidate = mStrips[j];
                if (candidate.pos.x == me.pos.x) continue;
                if (candidate.pos.x > me.pos.x + step) break;
                if (StripRepr::overlapY(me, candidate) && next(candidate) == Special::UNTOUCHED) {
                    next(candidate) = Special::TOUCHED;
                    next(me) = int16_t(j);
                    break;
                }
            }
        }
    }
}
