#ifndef FMO_DESKTOP_ARGS_HPP
#define FMO_DESKTOP_ARGS_HPP

#include "parser.hpp"

/// Processes and verifies command-line arguments.
struct Args {
    Args();

    std::vector<std::string> inputs; ///< paths to video files to use as inputs
    std::vector<std::string> gts;    ///< paths to ground truth text files, enables evaluation
    int camera = -1;                 ///< camera ID to use as input
    int stopAt = -1;                 ///< frame to pause at
    bool stopOnFn = false;           ///< pause when a false negative is encountered
    bool stopOnFp = false;           ///< pause when a false positive is encountered
    std::string out;                 ///< path to output evaluation results file
    std::string baseline;            ///< path to previously saved results file, enables comparison

private:
    Parser mParser;
};

#endif // FMO_DESKTOP_ARGS_HPP
