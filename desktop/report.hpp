#ifndef FMO_DESKTOP_REPORT_HPP
#define FMO_DESKTOP_REPORT_HPP

#include "evaluator.hpp"
#include <iosfwd>

struct Report {
    Report(const Results& results, const Results& baseline, float seconds);

    void write(std::ostream& out);

    void save(const std::string& directory);

private:
    static void info(std::ostream& out, const Results& results, const Results& baseline,
                     float seconds);

    const Results* mResults;
    std::string mInfo;
};

#endif // FMO_DESKTOP_REPORT_HPP
