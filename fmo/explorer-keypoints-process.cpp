#include "explorer-impl.hpp"
#include "include-opencv.hpp"
#include <fmo/algebra.hpp>
#include <fmo/assert.hpp>

namespace fmo {
    namespace {
        constexpr int int16_max = std::numeric_limits<int16_t>::max();
    }

    void Explorer::Impl::processKeypoints() {
        // reset
        mHaveObject = false;
        mComponents.clear();
        FMO_ASSERT(!mLevels.empty(), "no levels");
        int step = mLevels[0].step;

        FMO_ASSERT(mKeypoints.size() < size_t(int16_max), "too many keypoints");
        int numKeypoints = int(mKeypoints.size());
        // not tested: it is assumed that the keypoints are sorted by x coordinate

        for (int i = 0; i < numKeypoints; i++) {
            Keypoint& me = mKeypoints[i];

            // create new components for previously untouched keypoints
            if (me.special == Keypoint::UNTOUCHED) {
                mComponents.emplace_back(int16_t(i));
            }

            // find next keypoint
            me.special = Keypoint::END;
            for (int j = i + 1; j < numKeypoints; j++) {
                Keypoint& candidate = mKeypoints[j];
                int dx = candidate.x - me.x;
                if (dx > step) break;
                int dy = fmo::abs(candidate.y - me.y);
                if (dy < me.halfHeight + candidate.halfHeight) {
                    candidate.special = Keypoint::TOUCHED;
                    me.special = int16_t(j);
                    break;
                }
            }
        }
    }
}
