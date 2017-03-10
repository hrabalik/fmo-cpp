#ifndef FMO_DESKTOP_EVALUATOR_HPP
#define FMO_DESKTOP_EVALUATOR_HPP

#include "frameset.hpp"
#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>
#include <forward_list>
#include <map>

enum class Evaluation { TP = 0, TN = 1, FP = 2, FN = 3 };
inline bool good(Evaluation r) { return int(r) < 2; }

/// Responsible for storing and loading evaluation statistics.
struct Results {
    using File = std::vector<Evaluation>;

    /// Provides access to data regarding a specific file. A new data structure is created. If a
    /// structure with the given name already exists, an exception is thrown.
    File& newFile(const std::string& name);

    /// Provides access to data regatding a specific file. If there is no data, a reference to an
    /// empty data structure is returned.
    const File& getFile(const std::string& name) const;

private:
    // data
    std::forward_list<File> mList;
    std::map<std::string, File*> mMap;
};

/// Responsible for calculating frame statistics for a single input file.
struct Evaluator {
    static constexpr double IOU_THRESHOLD = 0.6;

    Evaluator(Results& results, const std::string& gtFilename, fmo::Dims dims);

    /// Decides whether the algorithm has been successful by analyzing the point set it has
    /// provided.
    Evaluation evaluateFrame(const fmo::PointSet& ps, int frameNum);

    /// Provide the ground truth at the specified frame.
    const fmo::PointSet& groundTruth(int frameNum) const { return mGt.get(frameNum - 1); }

private:
    // data
    int mFrameNum = 0;
    Results::File* mResults;
    FrameSet mGt;
    std::string mName;
};

#endif // FMO_DESKTOP_EVALUATOR_HPP
