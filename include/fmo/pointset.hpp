#ifndef FMO_POINTSET_HPP
#define FMO_POINTSET_HPP

#include <algorithm>
#include <fmo/common.hpp>
#include <memory>
#include <string>
#include <vector>

namespace fmo {
    using PointSet = std::vector<Pos>; ///< A set of points in an image.

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
