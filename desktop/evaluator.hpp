#ifndef FMO_DESKTOP_EVALUATOR_HPP
#define FMO_DESKTOP_EVALUATOR_HPP

#include "frameset.hpp"
#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>
#include <forward_list>
#include <iosfwd>
#include <map>

enum class Evaluation { TP = 0, TN = 1, FP = 2, FN = 3 };
enum class Comparison { NONE, SAME, IMPROVEMENT, REGRESSION };

struct EvalResult {
    Evaluation eval;
    Comparison comp;

    std::string str() const;
};

inline bool good(Evaluation r) { return int(r) < 2; }

/// Responsible for storing and loading evaluation statistics.
struct Results {
    using File = std::vector<Evaluation>;
    using map_t = std::map<std::string, File*>;
    using const_iterator = map_t::const_iterator;
    using size_type = map_t::size_type;
    Results() = default;

    /// Provides access to data regarding a specific file. A new data structure is created. If a
    /// structure with the given name already exists, an exception is thrown.
    File& newFile(const std::string& name);

    /// Provides access to data regatding a specific file. If there is no data, a reference to an
    /// empty data structure is returned.
    const File& getFile(const std::string& name) const;

    /// Loads results from file, previously saved with the save() method.
    void load(const std::string& file);

    /// Iterates over files in the results.
    const_iterator begin() const { return mMap.begin(); }

    /// Iterates over files in the results.
    const_iterator end() const { return mMap.end(); }

    /// Provides the number of files.
    size_type size() const { return mMap.size(); }

    /// Checks whether there are any files.
    bool empty() const { return mMap.empty(); }

private:
    // data
    std::forward_list<File> mList;
    std::map<std::string, File*> mMap;
};

/// Responsible for calculating frame statistics for a single input file.
struct Evaluator {
    static constexpr double IOU_THRESHOLD = 0.6;

    ~Evaluator();

    Evaluator(const std::string& gtFilename, fmo::Dims dims, Results& results,
              const Results& baseline);

    /// Decides whether the algorithm has been successful by analyzing the point set it has
    /// provided.
    EvalResult evaluateFrame(const fmo::PointSet& ps, int frameNum);

    /// Provide the ground truth at the specified frame.
    const fmo::PointSet& groundTruth(int frameNum) const { return mGt.get(frameNum - 1); }

    /// Provides the number of frames in the ground truth file.
    int numFrames() { return mGt.numFrames(); }

    /// Provides the evaluation result that was last returned by evaluateFrame().
    const EvalResult& getResult() const { return mResult; }

private:
    // data
    int mFrameNum = 0;
    Results::File* mResults;
    const Results::File* mBaseline;
    FrameSet mGt;
    std::string mName;
    EvalResult mResult;
};

/// Extracts filename from path.
std::string extractFilename(const std::string& path);

/// Extracts name of sequence from path.
std::string extractSequenceName(const std::string& path);

#endif // FMO_DESKTOP_EVALUATOR_HPP
