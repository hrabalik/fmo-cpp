#include "args.hpp"
#include <iostream>

namespace {
    using doc_t = const char* const;
    doc_t inputDoc = "<path> Path to an input video file. Can be used multiple times. Must not be "
                     "used with --camera.";
    doc_t gtDoc = "<path> Text file containing ground truth data. Using this option enables "
                  "quality evaluation. If used at all, this option must be used as many times as "
                  "--file. Use --out to save evaluation results.";
    doc_t cameraDoc = "<int> Input camera device ID. When this option is used, stream from the "
                      "specified camera will be used as input. Using ID 0 selects the default "
                      "camera, if available. Must not be used with --input, --wait, --fast, "
                      "--frame, --pause.";
    doc_t recordDirDoc = "<dir> Output directory to save video to. A new video file will be "
                         "created, storing the unmodified input video. The name of the video file "
                         "will be determined by system time. The directory must exist.";
    doc_t pauseOnFpDoc = "Playback will pause whenever a detection is deemed a false positive. "
                         "Must be used with --gt.";
    doc_t pauseOnFnDoc = "Playback will pause whenever a detection is deemed a false negative. "
                         "Must be used with --gt.";
    doc_t evalDirDoc = "<dir> Output directory to save evaluation results to. Must be used with "
                       "--gt.";
    doc_t baselineDoc = "<path> File with previously saved results (via --out) for comparison. "
                        "When used, the playback will pause to demonstrate where the results "
                        "differ. Must be used with --gt.";
    doc_t includeDoc = "<path> File with additional command-line arguments. The format is the same "
                       "as when specifying parameters on the command line. Whitespace such as tabs "
                       "and endlines is allowed.";
    doc_t pausedDoc = "Playback will be paused on the first frame. Shorthand for --frame 1. Must "
                      "not be used with --camera.";
    doc_t frameDoc = "<frame> Playback will be paused on the specified frame number. If there are "
                     "multiple input files, playback will be paused in each file that contains a "
                     "frame with the specified frame number. Must not be used with --camera.";
    doc_t fastDoc = "Sets the maximum playback speed. Shorthand for --wait 0. Must not be used "
                    "with --camera, --headless.";
    doc_t waitDoc = "<ms> Specifies the frame time in milliseconds, allowing for slow playback. "
                    "Must not be used with --camera, --headless.";
    doc_t headlessDoc = "Don't draw any GUI unless the playback is paused.";
    doc_t helpDoc = "Display help.";
}

Args::Args(int argc, char** argv) {
    // add commands
    mParser.add("--input", inputDoc, [this](const std::string& path) { inputs.push_back(path); });
    mParser.add("--gt", gtDoc, [this](const std::string& path) { gts.push_back(path); });
    mParser.add("--camera", cameraDoc, [this](int id) { camera = id; });
    mParser.add("--record-dir", recordDirDoc, [this](const std::string& dir) { recordDir = dir; });
    mParser.add("--pause-on-fp", pauseOnFpDoc, [this]() { pauseOnFp = true; });
    mParser.add("--pause-on-fn", pauseOnFnDoc, [this]() { pauseOnFn = true; });
    mParser.add("--eval-dir", evalDirDoc, [this](const std::string& path) { evalDir = path; });
    mParser.add("--baseline", baselineDoc, [this](const std::string& path) { baseline = path; });
    mParser.add("--include", includeDoc, [this](const std::string& path) { mParser.parse(path); });
    mParser.add("--paused", pausedDoc, [this]() { frame = 1; });
    mParser.add("--frame", frameDoc, [this](int frameNum) { frame = frameNum; });
    mParser.add("--fast", fastDoc, [this]() { wait = 0; });
    mParser.add("--wait", waitDoc, [this](int ms) { wait = ms; });
    mParser.add("--headless", headlessDoc, [this]() { headless = true; });
    mParser.add("--help", helpDoc, [this]() { help = true; });

    // parse command-line
    mParser.parse(argc, argv);

    // if requested, display help and exit
    if (help) {
        mParser.printHelp();
        std::exit(-1);
    }

    // validate parameters, throw if there is trouble
    validate();
}

void Args::validate() const {
    if (inputs.empty() && camera == -1) {
        throw std::runtime_error("one of --input, --camera must be specified");
    }
    if (camera != -1) {
        if (!inputs.empty()) { throw std::runtime_error("--camera cannot be used with --input"); }
        if (wait != -1) {
            throw std::runtime_error("--camera cannot be used with --wait or --fast");
        }
        if (frame != -1) {
            throw std::runtime_error("--camera canot be used with --frame or --pause");
        }
    }
    if (!gts.empty()) {
        if (gts.size() != inputs.size()) {
            std::cerr << "have " << inputs.size() << " inputs and " << gts.size() << " gts\n";
            throw std::runtime_error("there must be one --gt for each --input");
        }
    }
    if (gts.empty()) {
        if (pauseOnFn || pauseOnFp) {
            throw std::runtime_error("--pause-on-.. must be used with --gt");
        }
        if (!evalDir.empty()) { throw std::runtime_error("--eval-dir must be used with --gt"); }
        if (!baseline.empty()) { throw std::runtime_error("--baseline must be used with --gt"); }
    }
    if (headless && wait != -1) {
        throw std::runtime_error("--headless cannot be used with --wait or --fast");
    }
}
