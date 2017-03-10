#ifndef FMO_DESKTOP_EVALUATOR_HPP
#define FMO_DESKTOP_EVALUATOR_HPP

#include "frameset.hpp"
#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>

enum class Result { TP, TN, FP, FN };

/// Responsible for calculating frame statistics for a single input file.
struct Evaluator {
    static constexpr double IOU_THRESHOLD = 0.6;

    ~Evaluator();
    Evaluator(const std::string& gtFilename, fmo::Dims dims);

    /// Decides whether the algorithm has been successful by analyzing the point set it has
    /// provided.
    Result evaluateFrame(const fmo::PointSet& ps, int frameNum);

    /// Provide the ground truth at the specified frame.
    const fmo::PointSet& groundTruth(int frameNum) const { return mGt.get(frameNum - 1); }

    /// Provides the number of instances of a specific kind of result.
    int count(Result r) const { return mCount[int(r)]; }

private:
    friend struct Aggregator;

    // data
    int mCount[4] = {0, 0, 0, 0};
    int mFrameNum = 0;
    std::vector<Result> mResults;
    FrameSet mGt;
    std::string mGtName;
};

/// Responsible for calculating frame statistics for all input file.
struct Aggregator {
private:
    struct File {
        std::string name;
        std::vector<Result> results;
    };

    // data
    std::vector<int> todo;
};

#endif // FMO_DESKTOP_EVALUATOR_HPP
