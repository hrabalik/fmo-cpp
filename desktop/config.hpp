#ifndef FMO_CONFIG_HPP
#define FMO_CONFIG_HPP

#include <string>

/// References to current values of all configurable parameters.
struct Config {
    std::string& input;     ///< File to be used as input.
    int& camera;            ///< Camera ID to be used as input.
    std::string& gt;        ///< Path to ground truth file.
    std::string& recordDir; ///< Directory to store recordings to.
    int& wait;              ///< Additional delay between frames.
};

/// Provides the configuration singleton.
const Config& getConfig();

/// Updates the configured values by reading command-line inputs.
void readConfigFromCommandLine(int argc, char** argv);

#endif // FMO_CONFIG_HPP
