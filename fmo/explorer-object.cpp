#include "explorer-impl.hpp"
#include <algorithm>
#include <limits>

namespace fmo {
    namespace {
        constexpr int int_min = std::numeric_limits<int>::min();
        constexpr int int_max = std::numeric_limits<int>::max();
    }

    void Explorer::Impl::findObjects() {
        // reorder trajectories by score, best score will be at the front
        std::sort(begin(mTrajectories), end(mTrajectories),
                  [](const Trajectory& l, const Trajectory& r) { return l.score > r.score; });

        mObjects.clear();
        mRejected.clear();
        for (const auto& traj : mTrajectories) {
            // ignore all trajectories with zero score
            if (traj.score == 0) break;

            // test if the object is interesting
            if (isObject(traj)) {
                mObjects.push_back(&traj);
                break; // assume a single interesting object
            } else {
                mRejected.push_back(&traj);
            }
        }
    }

    bool Explorer::Impl::isObject(const Trajectory& traj) const {
        // find the range of x-coordinates of strips present in the difference images
        auto range1 = findTrajectoryRangeInDiff(traj, mLevel, mLevel.diff1);
        auto range2 = findTrajectoryRangeInDiff(traj, mLevel, mLevel.diff2);

        // condition: both diffs must have *some* strips present
        if (range1.first == int_max || range2.first == int_max) return false;

        // force left-to-right direction
        int xMin = mStrips[mComponents[traj.first].first].x;
        if (range1.first != xMin) {
            // force left-to-right by swapping ranges
            std::swap(range1, range2);
        }

        // condition: leftmost strip must be in range1
        if (range1.first != xMin) return false;

        // condition: rightmost strip must be in range2
        int xMax = mStrips[mComponents[traj.last].last].x;
        if (range2.second != xMax) return false;

        // condition: range1 must end a significant distance away from rightmost strip
        int minMotion = int(mCfg.minMotion * (xMax - xMin));
        if (xMax - range1.second < minMotion) return false;

        // condition: range2 must start a significant distance away from lefmost strip
        if (range2.first - xMin < minMotion) return false;

        return true;
    }

    std::pair<int, int> Explorer::Impl::findTrajectoryRangeInDiff(const Trajectory& traj,
                                                                  const Level& level,
                                                                  const Mat& diff) const {
        int step = level.step;
        int halfStep = step / 2;
        const uint8_t* data = diff.data();
        int skip = int(diff.skip());
        int first = int_max;
        int last = int_min;

        // iterate over all strips in trajectory
        int compIdx = traj.first;
        while (compIdx != Component::NO_COMPONENT) {
            const Component& comp = mComponents[compIdx];
            int stripIdx = comp.first;
            while (stripIdx != Strip::END) {
                const Strip& strip = mStrips[stripIdx];
                int col = (strip.x - halfStep) / step;
                int row = (strip.y - halfStep) / step;
                uint8_t val = *(data + (row * skip + col));

                // if the center of the strip is in the difference image
                if (val != 0) {
                    // update first, last
                    first = std::min(first, int(strip.x));
                    last = std::max(last, int(strip.x));
                }
                stripIdx = strip.special;
            }
            compIdx = comp.next;
        }

        return std::pair<int, int>{first, last};
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
