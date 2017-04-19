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
            float centerDistance = length(motion) / sumOfHalfLengths;
            if (centerDistance < 0.5f) {
                // minimum center motion: discard if too close
                return inf;
            }

            float dist1 = std::min(length(o1.endR - o2.endL), length(o2.endR - o1.endL));
            float dist2 = std::min(length(o1.endL - o2.endL), length(o2.endR - o1.endR));
            float distance = std::min(dist1, dist2) / sumOfHalfLengths;
            if (distance > mCfg.matchDistanceMax) {
                // maximum distance: discard if too far away
                return inf;
            }

            NormVector motionDirection{motion};
            NormVector motionDirectionP = perpendicular(motionDirection);
            float sin1 = std::abs(dot(motionDirectionP, o1.direction));
            float sin2 = std::abs(dot(motionDirectionP, o2.direction));
            float angle = std::max(sin1, sin2);
            bool testAngle = o1.aspect + o2.aspect > 3.f;
            if (testAngle && angle > mCfg.matchAngleMax) {
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

        int ends[2] = {int(mObjects[0].size()), int(mObjects[1].size())};
        mCache.matches.clear();
        mCache.matches.reserve(ends[0] * ends[1]);

        for (int i = 0; i < ends[0]; i++) {
            for (int j = 0; j < ends[1]; j++) {
                float aScore = score(mObjects[0][i], mObjects[1][j]);
                if (aScore < inf) {
                    Match m{aScore, {int16_t(i), int16_t(j)}};
                    mCache.matches.push_back(m);
                }
            }
        }

        while (!mCache.matches.empty()) {
            // select the best match
            float bestScore = inf;
            Match* bestMatch = nullptr;
            for (auto& match : mCache.matches) {
                if (match.score < bestScore) {
                    bestScore = match.score;
                    bestMatch = &match;
                }
            }
            Match selected = *bestMatch;

            // remove matches that involve the newly matched objects
            auto last = std::remove_if(begin(mCache.matches), end(mCache.matches),
                                       [this, selected](const Match& m) {
                                           return m.objects[0] == selected.objects[0] ||
                                                  m.objects[1] == selected.objects[1];
                                       });
            mCache.matches.erase(last, end(mCache.matches));

            // save the match
            mObjects[0][selected.objects[0]].prev = selected.objects[1];
        }
    }
}
