#include <fmo/benchmark.hpp>
#include <fmo/stats.hpp>

namespace {
    void log(log_t logFunc, const char* cStr) {
        logFunc(cStr);
    }

    template<typename Arg1, typename... Args>
    void log(log_t logFunc, const char* format, Arg1 arg1, Args... args) {
        char buf[81];
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wformat-security"
        snprintf(buf, sizeof(buf), format, arg1, args...);
#   pragma GCC diagnostic pop
        logFunc(buf);
    }
}

Registry& Registry::get() {
    static Registry instance;
    return instance;
}

void Registry::runAll(log_t logFunc, stop_t stopFunc) const {
    fmo::SectionStats stats;

    try {
        log(logFunc, "Benchmark started.\n");

        for (auto func : mFuncs) {
            stats.reset();
            bool updated = false;

            while (!updated && !stopFunc()) {
                stats.start();
                func.second();
                updated = stats.stop();
            }

            if (stopFunc()) {
                throw std::runtime_error("stopped");
            }

            auto q = stats.quantilesMs();
            log(logFunc, "%s: %.2f / %.1f / %.0f\n", func.first, q.q50, q.q95, q.q99);
        }

        log(logFunc, "Benchmark finished.\n\n");
    } catch (std::exception& e) {
        log(logFunc, "Benchmark interrupted: %s.\n\n", e.what());
    }
}

Benchmark::Benchmark(const char* name, bench_t func) {
    auto& reg = Registry::get();
    reg.add(name, func);
}


namespace {
    Benchmark FMO_UNIQUE_NAME{"stuff", []() {
    }};
}
