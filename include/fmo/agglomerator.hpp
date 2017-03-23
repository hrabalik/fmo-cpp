#ifndef FMO_AGGLOMERATOR_HPP
#define FMO_AGGLOMERATOR_HPP

#include <algorithm>
#include <cstdint>
#include <fmo/agglomerator.hpp>
#include <limits>
#include <vector>

namespace fmo {

    struct Agglomerator {
        using Id_t = int16_t;
        using Dist_t = float;

        static constexpr Id_t safetyMaxNumClusters = 100;
        static constexpr Dist_t infDist = std::numeric_limits<Dist_t>::max();

        /// Performs agglomerative clustering using a naive algorithm with O(n^3) time complexity
        /// and O(n^2) storage complexity. Clusters are merged greedily based on the provided
        /// distance function. Additionally, it is assumed that once a cluster is created by
        /// merging, the distance to all the other clusters cannot be derived from the previously
        /// calculated distances. Consequently, the problem is not an instance of single-linkage
        /// clustering. The merging operation effectively removes two clusters and adds a new one
        /// instead, therefore it is required that the distance function is cheap to calculate, with
        /// complexity O(1) even for non-trivial clusters.
        ///
        /// @param distanceFunc A funtion with signature Dist_t(Id_t i, Id_t j) or compatible that
        /// provides the distance between clusters i, j with time complexity O(1). Return infDist
        /// whenever merging the two clusters is impossible.
        /// @param mergeFunc A function with signature void(Id_t i, Id_t j) that merges clusters i
        /// and j into cluster i and invalidates cluster j.
        /// @param numClusters the initial number of clusters. Clusters are numbered 0 (inclusive)
        /// to numClusters (exclusive).
        template <typename DistanceFunc, typename MergeFunc>
        void operator()(DistanceFunc distanceFunc, MergeFunc mergeFunc, Id_t numClusters) {
            numClusters = std::min(numClusters, safetyMaxNumClusters);
            mIds.clear();
            mPairs.clear();

            auto addCluster = [&distanceFunc, this](Id_t i) {
                // calculate distances to all existing clusters
                for (auto j : mIds) {
                    Dist_t d = distanceFunc(i, j);
                    if (d != infDist) { mPairs.emplace_back(d, i, j); }
                }
                // add cluster
                mIds.push_back(i);
            };

            auto mergeClusters = [&mergeFunc, &addCluster, this](Id_t i, Id_t j) {
                // remove i, j from mIds
                {
                    auto last = std::remove_if(begin(mIds), end(mIds),
                                              [i, j](Id_t id) { return id == i || id == j; });
                    mIds.erase(last, end(mIds));
                }
                // remove i, j from mPairs
                {
                    auto last = std::remove_if(begin(mPairs), end(mPairs), [i, j](const Pair& r) {
                        return r.i == i || r.i == j || r.j == j || r.j == i;
                    });
                    mPairs.erase(last, end(mPairs));
                }
                // merge into i
                mergeFunc(i, j);
                // re-insert i
                addCluster(i);
            };

            auto findBestPair = [this]() {
                Dist_t bestDist = infDist;
                Pair* bestRel = nullptr;

                for (auto& pair : mPairs) {
                    if (pair.d < bestDist) {
                        bestDist = pair.d;
                        bestRel = &pair;
                    }
                }

                return bestRel;
            };

            // add initial clusters
            for (Id_t i = 0; i < numClusters; i++) { addCluster(i); }

            // merge until there's no viable pairs left
            while (!mPairs.empty()) {
                auto* pair = findBestPair();
                mergeClusters(pair->i, pair->j);
            }
        }

    private:
        /// For storing distance between clusters i and j.
        struct Pair {
            Dist_t d;
            Id_t i;
            Id_t j;

            Pair(Dist_t aD, Id_t aI, Id_t aJ) : d(aD), i(aI), j(aJ) {}
        };

        std::vector<Id_t> mIds;
        std::vector<Pair> mPairs;
    };
}

#endif // FMO_AGGLOMERATOR_HPP
