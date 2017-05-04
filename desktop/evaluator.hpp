#ifndef FMO_DESKTOP_EVALUATOR_HPP
#define FMO_DESKTOP_EVALUATOR_HPP

#include "frameset.hpp"
#include <array>
#include <fmo/algorithm.hpp>
#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>
#include <forward_list>
#include <iosfwd>
#include <map>

enum class Event { TP = 0, TN = 1, FP = 2, FN = 3 };
enum class Comparison { NONE, SAME, IMPROVEMENT, REGRESSION };

const std::string& eventName(Event e);

struct Evaluation {
    Evaluation() : mEvents{{0, 0, 0, 0}} {}
    int& operator[](Event e) { return mEvents[int(e)]; }
    int operator[](Event e) const { return mEvents[int(e)]; }
    void clear() { mEvents.fill(0); }

    void operator+=(const Evaluation& rhs) {
        for (int i = 0; i < 4; i++) { mEvents[i] += rhs.mEvents[i]; }
    }

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

/// Results of evaluation of a particular sequence.
struct FileResults {
    static constexpr double IOU_STORAGE_FACTOR = 1e3;
    std::vector<Evaluation> frames; ///< evaluation for each frame
    std::vector<int> iou;           ///< non-zero intersection-over-union values

    /// Clears all data.
    void clear() {
        frames.clear();
        iou.clear();
    }
};

/// Responsible for storing and loading evaluation statistics.
struct Results {
    using map_t = std::map<std::string, FileResults*>;
    using const_iterator = map_t::const_iterator;
    using size_type = map_t::size_type;
    Results() = default;

    /// Provides access to data regarding a specific file. A new data structure is created. If a
    /// structure with the given name already exists, an exception is thrown.
    FileResults& newFile(const std::string& name);

    /// Provides access to data regatding a specific file. If there is no data, a reference to an
    /// empty data structure is returned.
    const FileResults& getFile(const std::string& name) const;

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

    /// Calculate the histogram of intersection-over-union values.
    std::vector<int> makeIOUHistogram(int bins) const;

private:
    // data
    std::forward_list<FileResults> mList;
    std::map<std::string, FileResults*> mMap;
};

/// Responsible for calculating frame statistics for a single input file.
struct Evaluator {
    static constexpr int FRAME_OFFSET = -1;
    static constexpr double IOU_THRESHOLD = 0.5;

    ~Evaluator();

    Evaluator(const std::string& gtFilename, fmo::Dims dims, Results& results,
              const Results& baseline);

    /// Decides whether the algorithm has been successful by comparing the objects it has provided
    /// with the ground truth.
    EvalResult evaluateFrame(const fmo::Algorithm::Output& out, int frameNum);

    /// Provide the ground truth at the specified frame.
    const std::vector<fmo::PointSet>& groundTruth(int frameNum) const {
        return mGt.get(frameNum + FRAME_OFFSET);
    }

    /// Provides the number of frames in the ground truth file.
    int numFrames() { return mGt.numFrames(); }

    /// Provides the evaluation result that was last returned by evaluateFrame().
    const EvalResult& getResult() const { return mResult; }

private:
    // data
    int mFrameNum = 0;
    FileResults* mFile;
    const FileResults* mBaseline;
    FrameSet mGt;
    std::string mName;
    EvalResult mResult;
    fmo::PointSet mPointsCache;
    std::vector<double> mPsScores; ///< cached vector for storing IOUs of detected objects
    std::vector<double> mGtScores; ///< cached vector for storing IOUs of GT objects
};

/// Extracts filename from path.
std::string extractFilename(const std::string& path);

/// Extracts name of sequence from path.
std::string extractSequenceName(const std::string& path);

#endif // FMO_DESKTOP_EVALUATOR_HPP
