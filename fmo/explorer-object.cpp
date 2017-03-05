#include "explorer-impl.hpp"
#include <algorithm>
#include <limits>

namespace fmo {
    namespace {
        constexpr int int_min = std::numeric_limits<int>::min();
        constexpr int int_max = std::numeric_limits<int>::max();
    }

    void Explorer::Impl::findObject() {
        // reorder trajectories by score, best score will be at the end
        std::sort(begin(mTrajectories), end(mTrajectories),
            [] (const Trajectory& l, const Trajectory& r) { return l.score < r.score; });

        mObjects.clear();
        mRejectedObjects.clear();
        while (!mTrajectories.empty()) {
            const Trajectory& traj = mTrajectories.back();
            if (traj.score == 0) break;
            Bounds bounds = findBounds(traj);
            mObjects.push_back(bounds);
            break;
        }
    }

    auto Explorer::Impl::findBounds(const Trajectory& traj) -> Bounds {
        Bounds result;
        result.min = {int_max, int_max};
        result.max = {int_min, int_min};

        Component* comp = &mComponents[traj.first];
        Strip* firstStrip = &mStrips[comp->first];
        while (true) {
            Strip* strip = &mStrips[comp->first];
            while (true) {
                result.min.y = std::min(result.min.y, strip->y - strip->halfHeight);
                result.max.y = std::max(result.max.y, strip->y + strip->halfHeight);
                if (strip->special == Strip::END) break;
                strip = &mStrips[strip->special];
            }
            if (comp->next == Component::NO_COMPONENT) break;
            comp = &mComponents[comp->next];
        }
        Strip* lastStrip = &mStrips[comp->last];

        int halfWidth = mLevel.step / 2;
        result.min.x = firstStrip->x - halfWidth;
        result.max.x = lastStrip->x + halfWidth;
        return result;
    }
}
