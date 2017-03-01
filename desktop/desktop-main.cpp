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
    bool haveInput = cfg.input != "";
    bool haveOutDir = cfg.outDir != "";

    if ((haveInput ^ haveOutDir) == 0) {
        std::cerr << "Usage:  " TOSTR(FMO_BINARY_NAME) " ";
        std::cerr << "{--file <path> | --out-dir <path>} [--camera <num>]\n";
        std::cerr << "Options: --file   Input video file.\n";
        std::cerr << "         --outDir Output directory.\n";
        std::cerr << "         --camera Input device ID.\n";
        return -1;
    }

    // Mode mode = haveFile ? Mode::PLAYBACK : Mode::RECORD;

} catch (std::exception& e) { std::cerr << "An error occured: " << e.what() << '\n'; }
