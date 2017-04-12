#include "report.hpp"
#include "calendar.hpp"
#include <fmo/assert.hpp>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

Report::Report(const Results& results, const Results& baseline, const Args& args, float seconds) {
    mResults = &results;

    std::ostringstream out;
    info(out, results, baseline, args, seconds);
    mInfo = out.str();
}

void Report::write(std::ostream& out) { out << mInfo; }

void Report::save(const std::string& directory) {
    if (mResults->empty()) return;
    std::string fn = directory + '/' + safeTimestamp() + ".txt";
    std::ofstream out{fn, std::ios_base::out | std::ios_base::binary};

    if (!out) {
        std::cerr << "failed to open '" << fn << "'\n";
        throw std::runtime_error("failed to open file for writing results");
    }

    write(out);
    out << '\n';
    out << "/FMO/EVALUATION/V2/\n";
    out << mResults->size() << '\n';

    Event events[4] = {Event::FN, Event::FP, Event::TN, Event::TP};

    for (auto& entry : *mResults) {
        auto& name = entry.first;
        auto& file = *entry.second;
        out << name << ' ' << file.size() << '\n';

        for (int e = 0; e < 4; e++) {
            Event event = events[e];
            out << eventName(event);

            for (auto value : file) { out << ' ' << value[event]; }

            out << '\n';
        }
    }
}

void Report::info(std::ostream& out, const Results& results, const Results& baseline,
                  const Args& args, float seconds) {
    std::vector<std::string> fields;
    bool haveBase = false;
    Evaluation count;
    Evaluation countBase;
    double sum[2] = {0, 0};
    double sumBase[2] = {0, 0};
    int numFiles = 0;

    auto precision = [](Evaluation& count) {
        if (count[Event::FP] == 0) { return 1.; }
        int div = count[Event::TP] + count[Event::FP];
        return count[Event::TP] / double(div);
    };
    auto recall = [](Evaluation& count) {
        if (count[Event::FN] == 0) { return 1.; }
        int div = count[Event::TP] + count[Event::FN];
        return count[Event::TP] / double(div);
    };
    auto percent = [](std::ostream& out, double val) {
        out << std::fixed << std::setprecision(2) << (val * 100) << '%';
    };

    using stat_func_t = double(Evaluation&);
    stat_func_t* statFuncs[2] = {precision, recall};

    auto countStr = [&](Event event) {
        std::ostringstream out;
        int val = count[event];
        out << val;
        if (haveBase) {
            int delta = val - countBase[event];
            if (delta != 0) { out << " (" << std::showpos << delta << std::noshowpos << ')'; }
        }
        return out.str();
    };
    auto percentStr = [&](int i) {
        std::ostringstream out;
        double val = statFuncs[i](count);
        percent(out, val);
        if (haveBase) {
            double valBase = statFuncs[i](countBase);
            double delta = val - valBase;
            if (std::abs(delta) > 0.00005) {
                out << " (" << std::showpos;
                percent(out, delta);
                out << std::noshowpos << ')';
            }
        }
        return out.str();
    };
    auto addToAverage = [&](int i) {
        sum[i] += statFuncs[i](count);
        if (haveBase) { sumBase[i] += statFuncs[i](countBase); }
    };
    auto averageStr = [&](int i) {
        std::ostringstream out;
        double val = sum[i] / double(numFiles);
        percent(out, val);
        if (haveBase) {
            double valBase = sumBase[i] / double(numFiles);
            double delta = val - valBase;
            if (std::abs(delta) > 0.00005) {
                out << " (" << std::showpos;
                percent(out, delta);
                out << std::noshowpos << ')';
            }
        }
        return out.str();
    };

    fields.push_back("name");
    fields.push_back("tp");
    fields.push_back("tn");
    fields.push_back("fp");
    fields.push_back("fn");
    fields.push_back("precision");
    fields.push_back("recall");

    for (auto& entry : results) {
        auto& name = entry.first;
        auto& file = *entry.second;
        if (file.size() == 0) continue;
        auto& baseFile = baseline.getFile(name);
        haveBase = baseFile.size() == file.size();

        count.clear();
        for (auto eval : file) { count += eval; }
        if (haveBase) {
            countBase.clear();
            for (auto eval : baseFile) { countBase += eval; }
        }

        fields.push_back(name);
        fields.push_back(countStr(Event::TP));
        fields.push_back(countStr(Event::TN));
        fields.push_back(countStr(Event::FP));
        fields.push_back(countStr(Event::FN));
        fields.push_back(percentStr(0));
        fields.push_back(percentStr(1));

        addToAverage(0);
        addToAverage(1);

        numFiles++;
    }

    if (numFiles == 0) {
        // no entries -- quit
        return;
    }

    // calculate totals
    count.clear();
    countBase.clear();
    for (auto& entry : results) {
        auto& name = entry.first;
        auto& file = *entry.second;
        if (file.size() == 0) continue;
        auto& baseFile = baseline.getFile(name);
        haveBase = baseFile.size() == file.size();

        for (auto eval : file) { count += eval; }
        if (haveBase) {
            for (auto eval : baseFile) { countBase += eval; }
        }
    }

    fields.push_back("total");
    fields.push_back(countStr(Event::TP));
    fields.push_back(countStr(Event::TN));
    fields.push_back(countStr(Event::FP));
    fields.push_back(countStr(Event::FN));
    fields.push_back(percentStr(0));
    fields.push_back(percentStr(1));

    fields.push_back("average");
    fields.push_back("");
    fields.push_back("");
    fields.push_back("");
    fields.push_back("");
    fields.push_back(averageStr(0));
    fields.push_back(averageStr(1));

    constexpr int COLS = 7;
    int colSize[COLS] = {0, 0, 0, 0, 0, 0, 0};
    FMO_ASSERT(fields.size() % COLS == 0, "bad number of fields");

    auto hline = [&]() {
        for (int i = 0; i < colSize[0]; i++) { out << '-'; }
        for (int col = 1; col < COLS; col++) {
            out << '|';
            for (int i = 0; i < colSize[col]; i++) { out << '-'; }
        }
        out << '\n';
    };

    for (auto it = fields.begin(); it != fields.end();) {
        for (int col = 0; col < COLS; col++, it++) {
            colSize[col] = std::max(colSize[col], int(it->size()) + 1);
        }
    }

    out << "parameters: " << std::defaultfloat << std::setprecision(6);
    args.printParameters(out, ' ');
    out << "\n\n";
    out << "generated on: " << timestamp() << '\n';
    out << "evaluation time: " << std::fixed << std::setprecision(1) << seconds << " s\n";
    out << '\n';
    int row = 0;
    for (auto it = fields.begin(); it != fields.end(); row++) {
        out << std::setw(colSize[0]) << std::left << *it++ << std::right;
        for (int col = 1; col < COLS; col++, it++) { out << '|' << std::setw(colSize[col]) << *it; }
        out << '\n';
        if (row == 0) hline();
        if (row == numFiles) hline();
    }
}
