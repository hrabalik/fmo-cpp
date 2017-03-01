#include "config.hpp"
#include "param.hpp"
#include <fstream>
#include <utility>

namespace {
    /// Initializes the configuration subsystem and provides a Config object.
    std::pair<wtf::param_group&, Config&> config() {
        static wtf::param_group g;
        static wtf::param<std::string> file("file", "", g);
        static wtf::param<std::string> outDir("outDir", "", g);
        static wtf::param<int> camera("camera", 0, g);

        static Config c = {
            file.value, outDir.value, camera.value,
        };

        return {g, c};
    }

    /// Provides the parameter group singleton.
    wtf::param_group& paramGroup() { return config().first; }
}

const Config& getConfig() { return config().second; }

void readConfigFromFile(const std::string& fn) {
    std::ifstream ifs(fn);
    if (!ifs) throw std::runtime_error("Cannot open configuration file");
    wtf::stream_read_param(ifs, '\n', '=', paramGroup());
}

void readConfigFromCommandLine(int argc, char** argv) {
    wtf::command_line_read_param(argc, argv, paramGroup());
}
