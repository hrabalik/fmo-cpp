#ifndef FMO_DESKTOP_EVALUATOR_HPP
#define FMO_DESKTOP_EVALUATOR_HPP

#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>

enum class Result { TP, TN, FP, FN };

/// Responsible for calculating frame statistics for a single input file.
struct Evaluator {
    static constexpr double IOU_THRESHOLD = 0.6;

    /// Decides whether the algorithm has been successful by analyzing the point set it has
    /// provided.
    Result eval(const fmo::PointSet& ps, const fmo::PointSet& gt);

    /// Provides the number of instances of a specific kind of result.
    int count(Result r) const { return mCount[int(r)]; }

private:
    friend struct Aggregator;

    // data
    int mCount[4] = {0, 0, 0, 0};
    std::vector<Result> mResults;
};

/// Responsible for calculating frame statistics for all input file.
struct Aggregator {
private:
    struct File {
        std::string name;
        std::vector<Result> results;
    };

    //data
    std::vector<int> todo;
};

#endif // FMO_DESKTOP_EVALUATOR_HPP
