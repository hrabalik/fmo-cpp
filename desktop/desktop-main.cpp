#include "config.hpp"
#include <iostream>

#define TOSTR_INNER(x) #x
#define TOSTR(x) TOSTR_INNER(x)

enum class Mode {
    UNKNOWN,
    RECORD,
    PLAYBACK,
};

int main(int argc, char** argv) try {
    readConfigFromCommandLine(argc, argv);

    auto& cfg = getConfig();
    bool haveFile = cfg.file != "";
    bool haveOutDir = cfg.outDir != "";

    if ((haveFile ^ haveOutDir) == 0) {
        std::cerr << "Usage:  " TOSTR(FMO_BINARY_NAME) " ";
        std::cerr << "--file=<path> | --outDir=<path> [--camera=<num>]\n";
        std::cerr << "Options: --file   Specifies input video file.\n";
        std::cerr << "         --outDir Specifies output directory.\n";
        std::cerr << "         --camera Specifies input device ID.\n";
        return -1;
    }

    // Mode mode = haveFile ? Mode::PLAYBACK : Mode::RECORD;

} catch (std::exception& e) { std::cerr << "An error occured: " << e.what() << '\n'; }
