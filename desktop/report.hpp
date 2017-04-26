#ifndef FMO_DESKTOP_REPORT_HPP
#define FMO_DESKTOP_REPORT_HPP

#include "args.hpp"
#include "evaluator.hpp"
#include <iosfwd>

struct Report {
    Report(const Results& results, const Results& baseline, const Args& args, float seconds);

    void write(std::ostream& out) const;

    void save(const std::string& directory) const;

    void saveScore(const std::string& file) const;

private:
    struct Stats {
        double avg[2];       // average precision (0) and recall (1)
        double total[2];     // overall precision (0) and recall (1)
        double f1ScoreBase;  // ditto for baseline
        double avgBase[2];   // ditto for baseline
        double totalBase[2]; // ditto for baseline
    };

    static void info(std::ostream& out, Stats& stats, const Results& results,
                     const Results& baseline, const Args& args, float seconds);

    const Results* mResults;
    std::string mInfo;
    Stats mStats;
};

#endif // FMO_DESKTOP_REPORT_HPP
