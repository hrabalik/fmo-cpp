#ifndef FMO_DESKTOP_FRAMESET_HPP
#define FMO_DESKTOP_FRAMESET_HPP

#include <fmo/pointset.hpp>

/// A set of PointSets, one for each frame in a video.
struct FrameSet {
    FrameSet() = default;

    /// Loads points from a file.
    void load(const std::string& filename, fmo::Dims dims);

    /// Acquires the point set at a given frame. If there is no such set a raference to an empty
    /// point set is returned.
    const fmo::PointSet& get(int frameNum) const;

    fmo::Dims dims() const { return mDims; }
    int numFrames() const { return (int)mFrames.size(); }

private:
    /// Acquires the point set at a given frame. The argument must be in range 1 to numFrames()
    /// inclusive.
    std::unique_ptr<fmo::PointSet>& at(int frameNum) { return mFrames.at(frameNum - 1); }

    /// Acquires the point set at a given frame. The argument must be in range 1 to numFrames()
    /// inclusive.
    const std::unique_ptr<fmo::PointSet>& at(int frameNum) const {
        return mFrames.at(frameNum - 1);
    }

    // data
    fmo::Dims mDims = {0, 0};
    std::vector<std::unique_ptr<fmo::PointSet>> mFrames;
};

#endif // FMO_DESKTOP_FRAMESET_HPP
