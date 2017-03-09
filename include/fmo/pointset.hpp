#ifndef FMO_POINTSET_HPP
#define FMO_POINTSET_HPP

#include <algorithm>
#include <fmo/common.hpp>
#include <memory>
#include <string>
#include <vector>

namespace fmo {
    using PointSet = std::vector<Pos>; ///< A set of points in an image.

    inline bool pointSetComp(const Pos& l, const Pos& r) {
        return l.y < r.y || (l.y == r.y && l.x < r.x);
    }

    /// Compares two point sets. Assumes that both point sets are sorted. For each point in s1 but
    /// not in s2, s1Extra is called. For each point in s2 but not in s1, s2Extra is called. For
    /// each point in both s1 and s2, match is called.
    template <typename Func1, typename Func2, typename Func3>
    void pointSetCompare(const fmo::PointSet& s1, const fmo::PointSet& s2, Func1 s1Extra,
                         Func2 s2Extra, Func3 match) {
        auto i1 = begin(s1);
        auto i1e = end(s1);
        auto i2 = begin(s2);
        auto i2e = end(s2);

        while (i1 != i1e && i2 != i2e) {
            if (fmo::pointSetComp(*i1, *i2)) {
                s1Extra(*i1);
                i1++;
            } else if (fmo::pointSetComp(*i2, *i1)) {
                s2Extra(*i2);
                i2++;
            } else {
                match(*i1);
                i1++;
                i2++;
            }
        }

        while (i1 != i1e) {
            s1Extra(*i1);
            i1++;
        }

        while (i2 != i2e) {
            s2Extra(*i2);
            i2++;
        }
    }

    /// A set of PointSets, one for each frame in a video.
    struct FrameSet {
        FrameSet() = default;

        /// Loads points from a file.
        void load(const std::string& filename);

        const PointSet& get(int n) const {
            static const PointSet empty;
            auto first = mFrames.data();
            auto last = first + mFrames.size();
            auto* ptr = find(first, last, n);
            if (ptr == nullptr) return empty;
            return *ptr;
        }

        Dims dims() const { return mDims; }

    private:
        struct Frame {
            int n;
            PointSet set;
        };

        static const PointSet* find(const Frame* first, const Frame* last, int n) {
            if (first >= last) return nullptr;
            auto mid = first + (last - first) / 2;
            if (n < mid->n) return find(first, mid, n);
            if (n > mid->n)
                return find(mid + 1, last, n);
            else
                return &mid->set;
        }

        // data
        Dims mDims = {0, 0};
        int mNumFrames = 0;
        std::vector<Frame> mFrames;
    };
}

#endif
