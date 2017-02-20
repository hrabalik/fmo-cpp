#include "../catch/catch.hpp"
#include <fmo/benchmark.hpp>
#include <iostream>

SCENARIO("running benchmarks", "[.][perf]") {
    fmo::Registry::get().runAll([](const char* cStr) { std::cout << cStr; },
                                []() { return false; });
}
