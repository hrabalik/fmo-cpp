#include "../include-opencv.hpp"
#include "explorer.hpp"
#include <algorithm>
#include <fmo/processing.hpp>
#include <fmo/region.hpp>
#include <limits>
#include <type_traits>

namespace fmo {
    namespace {
        constexpr int BOUNDS_MIN = std::numeric_limits<int16_t>::min();
        constexpr int BOUNDS_MAX = std::numeric_limits<int16_t>::max();
    }

    void ExplorerV2::findObjects() {
        mObjects.clear();

        // sort valid clusters descending by total length
        auto& sortClusters = mCache.sortClusters;
        sortClusters.clear();

        for (auto& cluster : mClusters) {
            if (cluster.isInvalid()) continue;
            sortClusters.emplace_back(cluster.lengthTotal, &cluster);
        }

        std::sort(begin(sortClusters), end(sortClusters),
                  [](auto& l, auto& r) { return l.first > r.first; });

        // find the longest cluster that is considered an object
        for (auto& sortCluster : sortClusters) {
            if (isObject(*sortCluster.second)) {
                mObjects.push_back(sortCluster.second);
                break;
            } else {
                sortCluster.second->setInvalid(Cluster::NOT_AN_OBJECT);
            }
        }
    }

    bool ExplorerV2::isObject(Cluster& cluster) const {
        // find the bounding box enclosing strips present in the difference images
        cluster.bounds1 = findClusterBoundsInDiff(cluster, mLevel.diff1, mLevel.step);
        cluster.bounds2 = findClusterBoundsInDiff(cluster, mLevel.diff2, mLevel.step);

        // condition: both diffs must have *some* strips present
        if (cluster.bounds1.min.x == BOUNDS_MAX || cluster.bounds2.min.x == BOUNDS_MAX)
            return false;

        // force left-to-right direction
        int xMin = cluster.l.pos.x;
        if (cluster.bounds1.min.x != xMin) {
            // force left-to-right by swapping ranges
            std::swap(cluster.bounds1, cluster.bounds2);
        }

        // condition: leftmost strip must be in range1
        if (cluster.bounds1.min.x != xMin) return false;

        // condition: rightmost strip must be in range2
        int xMax = cluster.r.pos.x;
        if (cluster.bounds2.max.x != xMax) return false;

        // condition: range1 must end a significant distance away from rightmost strip
        int minMotion = int(mCfg.minMotion * (xMax - xMin));
        if (xMax - cluster.bounds1.max.x < minMotion) return false;

        // condition: range2 must start a significant distance away from lefmost strip
        if (cluster.bounds2.min.x - xMin < minMotion) return false;

        return true;
    }

    Bounds ExplorerV2::findClusterBoundsInDiff(const Cluster& cluster, const Mat& diff,
                                               int step) const {
        int halfStep = step / 2;
        const uint8_t* data = diff.data();
        int skip = int(diff.skip());
        Bounds result{{BOUNDS_MAX, BOUNDS_MAX}, {BOUNDS_MIN, BOUNDS_MIN}};

        // iterate over all strips in cluster
        int index = cluster.l.strip;
        while (index != Strip::END) {
            auto& strip = mStrips[index];
            int col = (strip.pos.x - halfStep) / step;
            int row = (strip.pos.y - halfStep) / step;
            uint8_t val = *(data + (row * skip + col));

            // if the center of the strip is in the difference image
            if (val != 0) {
                // update bounds
                result.min.x = std::min(result.min.x, int(strip.pos.x));
                result.min.y = std::min(result.min.y, int(strip.pos.y));
                result.max.x = std::max(result.max.x, int(strip.pos.x));
                result.max.y = std::max(result.max.y, int(strip.pos.y));
            }
            index = strip.special;
        }

        return result;
    }

    // auto ExplorerV2::findBounds(const Trajectory& traj) const -> Bounds {
    //     Bounds result;
    //     result.min = {BOUNDS_MAX, BOUNDS_MAX};
    //     result.max = {BOUNDS_MIN, BOUNDS_MIN};
    //
    //     const Component* comp = &mComponents[traj.first];
    //     const Strip* firstStrip = &mStrips[comp->first];
    //     while (true) {
    //         const Strip* strip = &mStrips[comp->first];
    //         while (true) {
    //             result.min.y = std::min(result.min.y, strip->y - strip->halfHeight);
    //             result.max.y = std::max(result.max.y, strip->y + strip->halfHeight);
    //             if (strip->special == Strip::END) break;
    //             strip = &mStrips[strip->special];
    //         }
    //         if (comp->next == Component::NO_COMPONENT) break;
    //         comp = &mComponents[comp->next];
    //     }
    //     const Strip* lastStrip = &mStrips[comp->last];
    //
    //     int halfWidth = mLevel.step / 2;
    //     result.min.x = firstStrip->x - halfWidth;
    //     result.max.x = lastStrip->x + halfWidth;
    //     return result;
    // }

    Bounds ExplorerV2::getObjectBounds() const {
        auto& obj = *mObjects[0];
        return grow(obj.bounds1, obj.bounds2);
    }

    void ExplorerV2::getObjectDetails(ObjectDetails& out) const {
        // find the bounding box enclosing the object
        auto& obj = *mObjects[0];
        out.bounds1 = obj.bounds1;
        out.bounds2 = obj.bounds2;

        // list object pixels
        getObjectPixels(out);
    }

    void fmo::ExplorerV2::getObjectPixels(ObjectDetails& out) const {
        out.points.clear();
        auto& obj = *mObjects[0];
        int step = mLevel.step;
        int halfStep = step / 2;
        int minX = std::max(obj.bounds1.min.x, obj.bounds2.min.x);
        int maxX = std::min(obj.bounds1.max.x, obj.bounds2.max.x);

        // iterate over all strips in cluster
        int index = obj.l.strip;
        while (index != Strip::END) {
            auto& strip = mStrips[index];

            // if the center of the strip is in both bounding boxes
            if (strip.pos.x >= minX && strip.pos.x <= maxX) {
                // put all pixels in the strip as object pixels
                int ye = strip.pos.y + strip.halfHeight;
                int xe = strip.pos.x + halfStep;

                for (int y = strip.pos.y - strip.halfHeight; y < ye; y++) {
                    for (int x = strip.pos.x - halfStep; x < xe; x++) {
                        out.points.push_back({x, y});
                    }
                }
            }

            index = strip.special;
        }
    }
}
