#ifndef FMO_DESKTOP_EVALUATOR_HPP
#define FMO_DESKTOP_EVALUATOR_HPP

#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>

/// Responsible for calculating frame statistics.
struct Evaluator {
    static constexpr double IOU_THRESHOLD = 0.6;

    enum class Result { TP, TN, FP, FN };

    /// Decides whether the algorithm has been successful by analyzing the point set it has
    /// provided.
    void eval(const fmo::PointSet& ps, const fmo::PointSet& gt);

    /// Provides the number of instances of a specific kind of result.
    int count(Result r) const { return mCount[int(r)]; }

private:
    // data
    int mCount[4] = {0, 0, 0, 0};
    std::vector<Result> mResults;
};

#endif // FMO_DESKTOP_EVALUATOR_HPP
