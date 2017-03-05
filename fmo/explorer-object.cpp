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

        // TODO reset success state
        mObject.good = false;
        while (!mTrajectories.empty()) {
            const Trajectory& traj = mTrajectories.back();
            if (traj.score == 0) break;
            findObjectBounds(traj);
            mObject.good = true;
            break;
        }
    }

    void Explorer::Impl::findObjectBounds(const Trajectory& traj) {
        mObject.min = {int_max, int_max};
        mObject.max = {int_min, int_min};

        Component* comp = &mComponents[traj.first];
        Strip* firstStrip = &mStrips[comp->first];
        while (true) {
            Strip* strip = &mStrips[comp->first];
            while (true) {
                mObject.min.y = std::min(mObject.min.y, strip->y - strip->halfHeight);
                mObject.max.y = std::max(mObject.max.y, strip->y + strip->halfHeight);
                if (strip->special == Strip::END) break;
                strip = &mStrips[strip->special];
            }
            if (comp->next == Component::NO_COMPONENT) break;
            comp = &mComponents[comp->next];
        }
        Strip* lastStrip = &mStrips[comp->last];

        int step = mLevels[0].step;
        mObject.min.x = firstStrip->x - step;
        mObject.max.x = lastStrip->x + step;
    }
}
