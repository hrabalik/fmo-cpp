#include "benchmark.hpp"

namespace {
    Benchmark FMO_UNIQUE_NAME {"stuff", []() {
        static int i = 0;
        if (i % 100 == 0) {
            benchLog("I was here! (%d)\n", i);
        }
        if (i == 500) {
            throw std::runtime_error("JAJAJAJAJA!!!");
        }
        i++;
    }};
}
