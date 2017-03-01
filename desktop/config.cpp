#include "config.hpp"
#include "param.hpp"
#include <fstream>
#include <utility>

namespace {
    /// Initializes the configuration subsystem and provides a Config object.
    std::pair<wtf::param_group&, Config&> config() {
        static wtf::param_group g;
        static wtf::param<std::string> input("input", "", g);
        static wtf::param<std::string> outDir("out-dir", "", g);
        static wtf::param<int> camera("camera", 0, g);

        static Config c = {
            input.value, outDir.value, camera.value,
        };

        return {g, c};
    }

    /// Provides the parameter group singleton.
    wtf::param_group& paramGroup() { return config().first; }
}

const Config& getConfig() { return config().second; }

void readConfigFromCommandLine(int argc, char** argv) {
    wtf::command_line_read_param(argc, argv, paramGroup());
}
