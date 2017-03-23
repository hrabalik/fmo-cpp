#include "explorer.hpp"

namespace fmo {
    namespace {
        int sqrDist(Pos p1, Pos p2) {
            int dx = p1.x - p2.x;
            int dy = p1.y - p2.y;
            return dx * dx + dy * dy;
        }
    }

    void ExplorerV2::findClusters() {
        // create initial clusters, one for each component that satisfies conditions
        auto makeInitialClusters = [this]() {
            mClusters.clear();
            auto& halfHeights = mCache.halfHeights;

            for (auto& comp : mComponents) {
                halfHeights.clear();
                int index = comp.first;
                Strip* firstStrip = &mStrips[index];
                Strip* strip = firstStrip;
                int numStrips = 0;

                while (true) {
                    numStrips++;
                    halfHeights.push_back(strip->halfHeight);
                    if (strip->special == Strip::END) break;
                    index = strip->special;
                    strip = &mStrips[index];
                }

                // analyze the half-heights
                std::sort(begin(halfHeights), end(halfHeights));
                auto q20 = begin(halfHeights) + (halfHeights.size() / 5);
                auto q50 = begin(halfHeights) + (halfHeights.size() / 2);
                auto q80 = begin(halfHeights) + (4 * halfHeights.size() / 5);

                // condition: q80/q20 of heights must not exceed a value
                if (float(*q80) / float(*q20) > mCfg.heightConsistencyInternal) continue;

                // add a new cluster
                mClusters.emplace_back();
                auto& cluster = mClusters.back();
                cluster.l.strip = comp.first;
                cluster.r.strip = index;
                cluster.l.pos = firstStrip->pos;
                cluster.r.pos = strip->pos;
                cluster.numStrips = numStrips;
                cluster.approxHeightMin = *q50;
                cluster.approxHeightMax = *q50;
                cluster.lengthSqr = sqrDist(cluster.l.pos, cluster.r.pos);
            }
        };

        makeInitialClusters();
    }
}
