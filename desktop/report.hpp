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
    static void info(std::ostream& out, double& scoreOut, const Results& results,
                     const Results& baseline, const Args& args, float seconds);

    const Results* mResults;
    std::string mInfo;
    double mScore;
};

#endif // FMO_DESKTOP_REPORT_HPP
