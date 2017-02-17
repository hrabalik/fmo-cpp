#define CATCH_CONFIG_COLOUR_NONE
#define CATCH_CONFIG_NOSTDOUT

#include "../catch/catch.hpp"

enum class BenchResult {
    GOOD,
    ERROR,
};

struct Registry {

};

BenchResult runCatch();
