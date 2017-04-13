#include "algorithm-median.hpp"
#include <limits>

namespace fmo {
    namespace {
        constexpr float inf = std::numeric_limits<float>::infinity();
    }

    void MedianV1::matchObjects() {
        auto score = [this](const Object& o1, const Object& o2) {
            float aspect = std::max(o1.aspect, o2.aspect) / std::min(o1.aspect, o2.aspect);
            if (aspect > mCfg.matchAspectMax) {
                // maximum aspect difference: discard if the shape differs
                return inf;
            }

            float area = std::max(o1.area, o2.area) / std::min(o1.area, o2.area);
            if (area > mCfg.matchAreaMax) {
                // maximum area difference: discard if size differs
                return inf;
            }

            float sumOfHalfLengths = o1.halfLen[0] + o2.halfLen[0];
            Vector motion = o2.center - o1.center;
            if (length(motion) < sumOfHalfLengths) {
                // minimum center motion: discard if too close
                return inf;
            }

            float dist1 = length(o1.endR - o2.endL);
            float dist2 = length(o2.endR - o1.endL);
            float distance = std::min(dist1, dist2) / sumOfHalfLengths;
            if (distance > mCfg.matchDistanceMax) {
                // maximum distance: discard if too far away
                return inf;
            }

            NormVector motionDirection{motion};
            NormVector motionDirectionP = perpendicular(motionDirection);
            float sin1 = dot(motionDirectionP, o1.direction);
            float sin2 = dot(motionDirectionP, o2.direction);
            float angle = std::max(sin1, sin2);
            if (angle > mCfg.matchAngleMax) {
                // maximum angle: discard if not on a line
                return inf;
            }

            float result = 0;
            result += mCfg.matchAspectWeight * aspect;
            result += mCfg.matchAreaWeight * area;
            result += mCfg.matchDistanceWeight * distance;
            result += mCfg.matchAngleWeight * angle;
            return result;
        };
    }
}
