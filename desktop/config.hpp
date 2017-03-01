#ifndef FMO_CONFIG_HPP
#define FMO_CONFIG_HPP

#include <string>

/// References to current values of all configurable parameters.
struct Config {
    std::string& file;   ///< File to be used as input.
    std::string& outDir; ///< Directory to store recordings to.
    int& camera;
};

/// Provides the configuration singleton.
const Config& getConfig();

/// Updates the configured values by reading a specified file.
void readConfigFromFile(const std::string& fn);

/// Updates the configured values by reading command-line inputs.
void readConfigFromCommandLine(int argc, char** argv);

#endif // FMO_CONFIG_HPP
