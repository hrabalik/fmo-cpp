#include "loop.hpp"
#include "report.hpp"
#include <iostream>

int main(int argc, char** argv) try {
    Status s{argc, argv};

    if (!s.args.baseline.empty()) { s.baseline.load(s.args.baseline); }
    if (s.haveCamera()) { s.args.inputs.emplace_back(); }

    for (size_t i = 0; !s.quit && i < s.args.inputs.size(); i++) {
        try {
            do {
                s.reload = false;
                processVideo(s, i);
            } while (s.reload);
        } catch (std::exception& e) {
            std::cerr << "while playing '" << s.args.inputs.at(i) << "'\n";
            throw e;
        }
    }

    Report report(s.results, s.baseline, s.timer.toc<fmo::TimeUnit::SEC, float>());
    report.write(std::cout);
    if (!s.args.evalDir.empty()) { report.save(s.args.evalDir); }
} catch (std::exception& e) {
    std::cerr << "error: " << e.what() << '\n';
    std::cerr << "tip: use --help to see a list of available commands\n";
    return -1;
}
