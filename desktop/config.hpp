#ifndef FMO_CONFIG_HPP
#define FMO_CONFIG_HPP

#include <string>

/// References to current values of all configurable parameters.
struct Config {
    std::string& input;  ///< File to be used as input.
    int& camera;
    std::string& outDir; ///< Directory to store recordings to.
    int& wait;
};

/// Provides the configuration singleton.
const Config& getConfig();

/// Updates the configured values by reading command-line inputs.
void readConfigFromCommandLine(int argc, char** argv);

#endif // FMO_CONFIG_HPP
