#include "../include-opencv.hpp"
#include "explorer.hpp"
#include <algorithm>
#include <fmo/algebra.hpp>
#include <fmo/assert.hpp>

namespace fmo {
    namespace {
        constexpr int int16_max = std::numeric_limits<int16_t>::max();
    }

    void ExplorerV3::findComponents() {
        // reset
        mComponents.clear();
        int step = mLevel.step;

        FMO_ASSERT(mStrips.size() < size_t(int16_max), "too many strips");
        int numStrips = int(mStrips.size());
        // not tested: it is assumed that the strips are sorted by x coordinate

        for (int i = 0; i < numStrips; i++) {
            Strip& me = mStrips[i];

            // create new components for previously untouched strips
            if (me.special == Strip::UNTOUCHED) { mComponents.emplace_back(int16_t(i)); }

            // find next strip
            me.special = Strip::END;
            for (int j = i + 1; j < numStrips; j++) {
                Strip& candidate = mStrips[j];
                if (Strip::inContact(me, candidate, step)) {
                    candidate.special = Strip::TOUCHED;
                    me.special = int16_t(j);
                    break;
                }
            }
        }
    }
}
