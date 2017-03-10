#ifndef FMO_DESKTOP_FRAMESET_HPP
#define FMO_DESKTOP_FRAMESET_HPP

#include <fmo/pointset.hpp>

/// A set of PointSets, one for each frame in a video.
struct FrameSet {
    FrameSet() = default;

    /// Loads points from a file.
    void load(const std::string& filename, fmo::Dims dims);

    const fmo::PointSet& get(int n) const {
        static const fmo::PointSet empty;
        auto first = mFrames.data();
        auto last = first + mFrames.size();
        auto* ptr = find(first, last, n);
        if (ptr == nullptr) return empty;
        return *ptr;
    }

    fmo::Dims dims() const { return mDims; }

private:
    struct Frame {
        int n;
        fmo::PointSet set;
    };

    static const fmo::PointSet* find(const Frame* first, const Frame* last, int n) {
        if (first >= last) return nullptr;
        auto mid = first + (last - first) / 2;
        if (n < mid->n) return find(first, mid, n);
        if (n > mid->n)
            return find(mid + 1, last, n);
        else
            return &mid->set;
    }

    // data
    fmo::Dims mDims = {0, 0};
    int mNumFrames = 0;
    std::vector<Frame> mFrames;
};

#endif // FMO_DESKTOP_FRAMESET_HPP
