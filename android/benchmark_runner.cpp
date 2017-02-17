#define CATCH_CONFIG_RUNNER

#include "benchmark.hpp"

namespace {
    struct {
        std::ostringstream out;
    } global;
}

namespace Catch {
    std::ostream& cout() {
        return global.out;
    }

    std::ostream& cerr() {
        return global.out;
    }
}

BenchResult runCatch() {
    // reset output
    global.out.str("");
    global.out.clear();

    // run Catch tests
    static Catch::Session session;
    int code = session.run();
    return (code == 0) ? BenchResult::GOOD : BenchResult::ERROR;
}
