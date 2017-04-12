#ifndef FMO_DESKTOP_EVALUATOR_HPP
#define FMO_DESKTOP_EVALUATOR_HPP

#include "frameset.hpp"
#include <array>
#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>
#include <forward_list>
#include <iosfwd>
#include <map>

enum class Event { TP = 0, TN = 1, FP = 2, FN = 3 };
enum class Comparison { NONE, SAME, IMPROVEMENT, REGRESSION };

const std::string& eventName(Event e);

struct Evaluation {
    int& operator[](Event e) { return mEvents[int(e)]; }
    int operator[](Event e) const { return mEvents[int(e)]; }
    void clear() { mEvents.fill(0); }

private:
    std::array<int, 4> mEvents;
};

struct EvalResult {
    Evaluation eval;
    Comparison comp;

    void clear() {
        eval.clear();
        comp = Comparison::NONE;
    }
    std::string str() const;
};

inline bool good(Evaluation r) { return r[Event::FN] + r[Event::FP] == 0; }
inline bool bad(Evaluation r) { return r[Event::TN] + r[Event::TP] == 0; }

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
    static constexpr double IOU_THRESHOLD = 0.0;

    ~Evaluator();

    Evaluator(const std::string& gtFilename, fmo::Dims dims, Results& results,
              const Results& baseline);

    /// Decides whether the algorithm has been successful by comparing the objects it has provided
    /// with the ground truth.
    EvalResult evaluateFrame(const std::vector<fmo::PointSet>& ps, int frameNum);

    /// Provide the ground truth at the specified frame.
    const std::vector<fmo::PointSet>& groundTruth(int frameNum) const {
        return mGt.get(frameNum - 1);
    }

    /// Provides the number of frames in the ground truth file.
    int numFrames() { return mGt.numFrames(); }

    /// Provides the evaluation result that was last returned by evaluateFrame().
    const EvalResult& getResult() const { return mResult; }

private:
    // data
    int mFrameNum = 0;
    Results::File* mFile;
    const Results::File* mBaseline;
    FrameSet mGt;
    std::string mName;
    EvalResult mResult;
    std::vector<double> mPsScores; ///< cached vector for storing IOUs of detected objects
    std::vector<double> mGtScores; ///< cached vector for storing IOUs of GT objects
};

/// Extracts filename from path.
std::string extractFilename(const std::string& path);

/// Extracts name of sequence from path.
std::string extractSequenceName(const std::string& path);

#endif // FMO_DESKTOP_EVALUATOR_HPP
