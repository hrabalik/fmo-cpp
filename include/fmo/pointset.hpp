#ifndef FMO_POINTSET_HPP
#define FMO_POINTSET_HPP

#include <algorithm>
#include <fmo/common.hpp>
#include <memory>
#include <string>
#include <vector>

namespace fmo {
    /// Comparison function for PointSet.
    inline bool pointSetComp(const Pos& l, const Pos& r) {
        return l.y < r.y || (l.y == r.y && l.x < r.x);
    }

    /// A set of points in an image. As an invariant, the set must be sorted according to
    /// pointSetComp.
    using PointSet = std::vector<Pos>;

    /// Compares two point sets. Assumes that both point sets are sorted. For each point in s1 but
    /// not in s2, extra1 is called. For each point in s2 but not in s1, extra2 is called. For each
    /// point in both s1 and s2, match is called.
    template <typename Func1, typename Func2, typename Func3>
    void pointSetCompare(const fmo::PointSet& s1, const fmo::PointSet& s2, Func1 extra1,
                         Func2 extra2, Func3 match) {
        auto i1 = begin(s1);
        auto i1e = end(s1);
        auto i2 = begin(s2);
        auto i2e = end(s2);

        while (i1 != i1e && i2 != i2e) {
            if (fmo::pointSetComp(*i1, *i2)) {
                extra1(*i1);
                i1++;
            } else if (fmo::pointSetComp(*i2, *i1)) {
                extra2(*i2);
                i2++;
            } else {
                match(*i1);
                i1++;
                i2++;
            }
        }

        while (i1 != i1e) {
            extra1(*i1);
            i1++;
        }

        while (i2 != i2e) {
            extra2(*i2);
            i2++;
        }
    }
}

#endif
