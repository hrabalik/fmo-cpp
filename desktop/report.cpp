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
    info(out, mStats, results, baseline, args, seconds);
    mInfo = out.str();
}

void Report::write(std::ostream& out) const { out << mInfo; }

void Report::save(const std::string& directory) const {
    if (mResults->empty()) return;
    std::string fn = directory + '/' + safeTimestamp() + ".txt";
    std::ofstream out{fn, std::ios_base::out | std::ios_base::binary};

    if (!out) {
        std::cerr << "failed to open '" << fn << "'\n";
        throw std::runtime_error("failed to open file for writing results");
    }

    write(out);
    out << '\n';
    out << "/FMO/EVALUATION/V3/\n";
    out << mResults->size() << '\n';

    Event events[4] = {Event::FN, Event::FP, Event::TN, Event::TP};

    for (auto& entry : *mResults) {
        auto& name = entry.first;
        auto& file = *entry.second;
        size_t numIOUs = file.iou.size();
        out << name << ' ' << file.frames.size() << ' ' << numIOUs << '\n';

        for (int e = 0; e < 4; e++) {
            Event event = events[e];
            out << eventName(event);
            for (auto eval : file.frames) { out << ' ' << eval[event]; }
            out << '\n';
        }

        if (numIOUs > 0) {
            out << "IOU";
            for (auto value : file.iou) { out << ' ' << value; }
            out << '\n';
        }
    }
}

void Report::saveScore(const std::string& file) const {
    std::ofstream out{file};
    auto print = [&out](double d) { out << std::fixed << std::setprecision(12) << d << '\n'; };
    print(mStats.avg[0]);
    print(mStats.avg[1]);
    print(mStats.total[0]);
    print(mStats.total[1]);
}

void Report::info(std::ostream& out, Stats& stats, const Results& results, const Results& baseline,
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

    auto countStrImpl = [](int val, int valBase) {
        std::ostringstream out;
        out << val;
        int delta = val - valBase;
        if (delta != 0) { out << " (" << std::showpos << delta << std::noshowpos << ')'; }
        return out.str();
    };
    auto percentStrImpl = [&](double val, double valBase) {
        std::ostringstream out;
        percent(out, val);
        double delta = val - valBase;
        if (std::abs(delta) > 5e-5) {
            out << " (" << std::showpos;
            percent(out, delta);
            out << std::noshowpos << ')';
        }
        return out.str();
    };
    auto countStr = [&](Event event) {
        int val = count[event];
        int valBase = haveBase ? countBase[event] : val;
        return countStrImpl(val, valBase);
    };
    auto percentStr = [&](int i) {
        double val = statFuncs[i](count);
        double valBase = haveBase ? statFuncs[i](countBase) : val;
        return percentStrImpl(val, valBase);
    };
    auto addToAverage = [&](int i) {
        sum[i] += statFuncs[i](count);
        if (haveBase) { sumBase[i] += statFuncs[i](countBase); }
    };
    auto totalStr = [&](int i) {
        stats.total[i] = statFuncs[i](count);
        stats.totalBase[i] = haveBase ? statFuncs[i](countBase) : stats.total[i];
        return percentStrImpl(stats.total[i], stats.totalBase[i]);
    };
    auto averageStr = [&](int i) {
        stats.avg[i] = sum[i] / double(numFiles);
        stats.avgBase[i] = haveBase ? (sumBase[i] / double(numFiles)) : stats.avg[i];
        return percentStrImpl(stats.avg[i], stats.avgBase[i]);
    };
    auto fScore = [](double* avg, double beta) {
        if (avg[0] <= 0 || avg[1] <= 0) return 0.;
        double betaSqr = beta * beta;
        return ((betaSqr + 1) * avg[0] * avg[1]) / ((betaSqr * avg[0]) + avg[1]);
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
        if (file.frames.size() == 0) continue;
        auto& baseFile = baseline.getFile(name);
        haveBase = baseFile.frames.size() == file.frames.size();

        count.clear();
        for (auto eval : file.frames) { count += eval; }
        if (haveBase) {
            countBase.clear();
            for (auto eval : baseFile.frames) { countBase += eval; }
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
        if (file.frames.size() == 0) continue;
        auto& baseFile = baseline.getFile(name);
        haveBase = baseFile.frames.size() == file.frames.size();

        for (auto eval : file.frames) { count += eval; }
        if (haveBase) {
            for (auto eval : baseFile.frames) { countBase += eval; }
        }
    }

    fields.push_back("total");
    fields.push_back(countStr(Event::TP));
    fields.push_back(countStr(Event::TN));
    fields.push_back(countStr(Event::FP));
    fields.push_back(countStr(Event::FN));
    fields.push_back(totalStr(0));
    fields.push_back(totalStr(1));

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

    constexpr int numBins = 10;
    auto hist = results.makeIOUHistogram(numBins);
    auto histBase = baseline.makeIOUHistogram(numBins);

    out << "parameters: " << std::defaultfloat << std::setprecision(6);
    args.printParameters(out, ' ');
    out << "\n\n";
    out << "generated on: " << timestamp() << '\n';
    out << "evaluation time: " << std::fixed << std::setprecision(1) << seconds << " s\n";
    out << "f_0.5 score: " << percentStrImpl(fScore(stats.avg, 0.5), fScore(stats.avgBase, 0.5))
        << '\n';
    out << "f_1.0 score: " << percentStrImpl(fScore(stats.avg, 1.0), fScore(stats.avgBase, 1.0))
        << '\n';
    out << "f_2.0 score: " << percentStrImpl(fScore(stats.avg, 2.0), fScore(stats.avgBase, 2.0))
        << '\n';
    out << "iou: ";
    for (int i = 0; i < numBins; i++) { out << countStrImpl(hist[i], histBase[i]) << " "; }
    out << "\n\n";
    int row = 0;
    for (auto it = fields.begin(); it != fields.end(); row++) {
        out << std::setw(colSize[0]) << std::left << *it++ << std::right;
        for (int col = 1; col < COLS; col++, it++) { out << '|' << std::setw(colSize[col]) << *it; }
        out << '\n';
        if (row == 0) hline();
        if (row == numFiles) hline();
    }
}
