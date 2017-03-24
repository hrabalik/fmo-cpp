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
    doc_t pauseFpDoc = "Playback will pause whenever a detection is deemed a false positive. Must "
                       "be used with --gt.";
    doc_t pauseFnDoc = "Playback will pause whenever a detection is deemed a false negative. Must "
                       "be used with --gt.";
    doc_t pauseRgDoc = "Playback will pause whenever a regression is detected, i.e. whenever a "
                       "frame is evaluated as false and baseline is true. Must be used with "
                       "--baseline.";
    doc_t pauseImDoc = "Playback will pause whenever an improvement is detected, i.e. whenever a "
                       "frame is evaluated as true and baseline is false. Must be used with "
                       "--baseline.";
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
    doc_t headlessDoc = "Don't draw any GUI unless the playback is paused. Must be used with --gt. "
                        "Must not be used with --wait, --fast.";
    doc_t demoDoc = "Force demo visualization method. This visualization method is preferred when "
                    "--camera is used.";
    doc_t debugDoc = "Force debug visualization method. This visualization method is preferred "
                     "when --input is used.";
    doc_t helpDoc = "Display help.";
    doc_t algorithmDoc = "<name> Specifies the name of the algorithm variant. Use --list to list "
                         "available algorithm names.";
    doc_t listDoc = "Display available algorithm names. Use --algorithm to select an algorithm.";
    doc_t paramDocI = "<int> Algorithm parameter.";
    doc_t paramDocF = "<float> Algorithm parameter.";
    doc_t paramDocUint8 = "<uint8> Algorithm parameter.";
}

Args::Args(int argc, char** argv)
    : inputs(),
      gts(),
      camera(-1),
      recordDir("."),
      pauseFn(false),
      pauseFp(false),
      pauseRg(false),
      pauseIm(false),
      evalDir(),
      baseline(),
      frame(-1),
      wait(-1),
      headless(false),
      demo(false),
      debug(false),
      help(false) {

    // add commands
    mParser.adds("--input", inputDoc, [this](const std::string& path) { inputs.push_back(path); });
    mParser.adds("--gt", gtDoc, [this](const std::string& path) { gts.push_back(path); });
    mParser.addi("--camera", cameraDoc, [this](int id) { camera = id; });
    mParser.adds("--record-dir", recordDirDoc, [this](const std::string& dir) { recordDir = dir; });
    mParser.addb("--pause-fp", pauseFpDoc, [this]() { pauseFp = true; });
    mParser.addb("--pause-fn", pauseFnDoc, [this]() { pauseFn = true; });
    mParser.addb("--pause-rg", pauseRgDoc, [this]() { pauseRg = true; });
    mParser.addb("--pause-im", pauseImDoc, [this]() { pauseIm = true; });
    mParser.adds("--eval-dir", evalDirDoc, [this](const std::string& path) { evalDir = path; });
    mParser.adds("--baseline", baselineDoc, [this](const std::string& path) { baseline = path; });
    mParser.adds("--include", includeDoc, [this](const std::string& path) { mParser.parse(path); });
    mParser.addb("--paused", pausedDoc, [this]() { frame = 1; });
    mParser.addi("--frame", frameDoc, [this](int frameNum) { frame = frameNum; });
    mParser.addb("--fast", fastDoc, [this]() { wait = 0; });
    mParser.addi("--wait", waitDoc, [this](int ms) { wait = ms; });
    mParser.addb("--headless", headlessDoc, [this]() { headless = true; });
    mParser.addb("--demo", demoDoc, [this]() { demo = true; });
    mParser.addb("--debug", debugDoc, [this]() { debug = true; });
    mParser.addb("--help", helpDoc, [this]() { help = true; });

    // add algorithm params
    mParser.adds("--algorithm", algorithmDoc, [this](auto& name) { params.name = name; });
    mParser.addb("--list", listDoc, [this]() { list = true; });
    mParser.addi("--p-thresh-gray", paramDocUint8,
                 [this](int thresh) { params.diff.threshGray = uint8_t(thresh); });
    mParser.addi("--p-thresh-bgr", paramDocUint8,
                 [this](int thresh) { params.diff.threshBgr = uint8_t(thresh); });
    mParser.addi("--p-thresh-yuv", paramDocUint8,
                 [this](int thresh) { params.diff.threshYuv = uint8_t(thresh); });
    mParser.addf("--p-min-gap", paramDocF, [this](float gap) { params.minGap = gap; });
    mParser.addi("--p-max-image-height", paramDocI,
                 [this](int height) { params.maxImageHeight = height; });
    mParser.addi("--p-min-strip-height", paramDocI,
                 [this](int height) { params.minStripHeight = height; });
    mParser.addi("--p-min-strips-in-component", paramDocI,
                 [this](int num) { params.minStripsInComponent = num; });
    mParser.addi("--p-min-strips-in-cluster", paramDocI,
                 [this](int num) { params.minStripsInCluster = num; });
    mParser.addf("--p-weight-height-ratio", paramDocF,
                 [this](float weight) { params.heightRatioWeight = weight; });
    mParser.addf("--p-weight-distance", paramDocF,
                 [this](float weight) { params.distanceWeight = weight; });
    mParser.addf("--p-weight-gaps", paramDocF,
                 [this](float weight) { params.gapsWeight = weight; });
    mParser.addf("--p-max-height-ratio-internal", paramDocF,
                 [this](float ratio) { params.maxHeightRatioInternal = ratio; });
    mParser.addf("--p-max-height-ratio-external", paramDocF,
                 [this](float ratio) { params.maxHeightRatioExternal = ratio; });
    mParser.addf("--p-max-distance", paramDocF, [this](float dist) { params.maxDistance = dist; });
    mParser.addf("--p-max-gaps-length", paramDocF,
                 [this](float length) { params.maxGapsLength = length; });
    mParser.addf("--p-min-motion", paramDocF, [this](float motion) { params.minMotion = motion; });

    // parse command-line
    mParser.parse(argc, argv);

    // if requested, display help and exit
    if (help) {
        mParser.printHelp();
        std::exit(-1);
    }

    // if requested, display list and exit
    if (list) {
        auto names = fmo::Algorithm::listFactories();
        for (auto& name : names) {
            std::cerr << name << '\n';
        }
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
        if (pauseFn || pauseFp) {
            throw std::runtime_error("--pause-fn|fp must be used with --gt");
        }
        if (!evalDir.empty()) { throw std::runtime_error("--eval-dir must be used with --gt"); }
        if (!baseline.empty()) { throw std::runtime_error("--baseline must be used with --gt"); }
        if (headless) { throw std::runtime_error("--headless must be used with --gt"); }
    }
    if (baseline.empty()) {
        if (pauseRg || pauseIm) {
            throw std::runtime_error("--pause-rg|im must be used with --baseline");
        }
    }
    if (demo && debug) { throw std::runtime_error("--demo must not be used with --debug"); }
    if (headless && wait != -1) {
        throw std::runtime_error("--headless cannot be used with --wait or --fast");
    }
}
