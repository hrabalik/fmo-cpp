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
    doc_t scoreFileDoc = "<file> File to write a numeric evaluation score to.";
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
    doc_t defaultsDoc = "Display default values for all parameters.";
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
      params(),
      mParser(),
      mHelp(false),
      mDefaults(false),
      mList(false) {

    // add commands
    mParser.add("--input", inputDoc, inputs);
    mParser.add("--gt", gtDoc, gts);
    mParser.add("--camera", cameraDoc, camera);
    mParser.add("--record-dir", recordDirDoc, recordDir);
    mParser.add("--pause-fp", pauseFpDoc, pauseFp);
    mParser.add("--pause-fn", pauseFnDoc, pauseFn);
    mParser.add("--pause-rg", pauseRgDoc, pauseRg);
    mParser.add("--pause-im", pauseImDoc, pauseIm);
    mParser.add("--eval-dir", evalDirDoc, evalDir);
    mParser.add("--score-file", scoreFileDoc, scoreFile);
    mParser.add("--baseline", baselineDoc, baseline);
    mParser.add("--include", includeDoc, [this](const std::string& path) { mParser.parse(path); });
    mParser.add("--paused", pausedDoc, [this]() { frame = 1; });
    mParser.add("--frame", frameDoc, frame);
    mParser.add("--fast", fastDoc, [this]() { wait = 0; });
    mParser.add("--wait", waitDoc, wait);
    mParser.add("--headless", headlessDoc, headless);
    mParser.add("--demo", demoDoc, demo);
    mParser.add("--debug", debugDoc, debug);
    mParser.add("--help", helpDoc, mHelp);
    mParser.add("--defaults", defaultsDoc, mDefaults);

    // add algorithm params
    mParser.add("--algorithm", algorithmDoc, params.name);
    mParser.add("--list", listDoc, mList);

    mParser.add("--p-thresh", paramDocUint8, params.diff.thresh);
    mParser.add("--p-thresh-min-noise", paramDocF, params.diff.noiseMin);
    mParser.add("--p-thresh-max-noise", paramDocF, params.diff.noiseMax);
    mParser.add("--p-max-gap-x", paramDocF, params.maxGapX);
    mParser.add("--p-min-gap-y", paramDocF, params.minGapY);
    mParser.add("--p-max-image-height", paramDocI, params.maxImageHeight);
    mParser.add("--p-min-strip-height", paramDocI, params.minStripHeight);
    mParser.add("--p-min-strips-in-object", paramDocI, params.minStripsInObject);
    mParser.add("--p-min-strip-area", paramDocF, params.minStripArea);
    mParser.add("--p-min-aspect", paramDocF, params.minAspect);
    mParser.add("--p-min-aspect-for-relevant-angle", paramDocF, params.minAspectForRelevantAngle);
    mParser.add("--p-match-aspect-max", paramDocF, params.matchAspectMax);
    mParser.add("--p-match-area-max", paramDocF, params.matchAreaMax);
    mParser.add("--p-match-distance-min", paramDocF, params.matchDistanceMin);
    mParser.add("--p-match-distance-max", paramDocF, params.matchDistanceMax);
    mParser.add("--p-match-angle-max", paramDocF, params.matchAngleMax);
    mParser.add("--p-match-aspect-weight", paramDocF, params.matchAspectWeight);
    mParser.add("--p-match-area-weight", paramDocF, params.matchAreaWeight);
    mParser.add("--p-match-distance-weight", paramDocF, params.matchDistanceWeight);
    mParser.add("--p-match-angle-weight", paramDocF, params.matchAngleWeight);
    mParser.add("--p-select-max-distance", paramDocF, params.selectMaxDistance);

    mParser.add("--p-min-strips-in-component", paramDocI, params.minStripsInComponent);
    mParser.add("--p-min-strips-in-cluster", paramDocI, params.minStripsInCluster);
    mParser.add("--p-min-cluster-length", paramDocF, params.minClusterLength);
    mParser.add("--p-weight-height-ratio", paramDocF, params.heightRatioWeight);
    mParser.add("--p-weight-distance", paramDocF, params.distanceWeight);
    mParser.add("--p-weight-gaps", paramDocF, params.gapsWeight);
    mParser.add("--p-max-height-ratio-internal", paramDocF, params.maxHeightRatioInternal);
    mParser.add("--p-max-height-ratio-external", paramDocF, params.maxHeightRatioExternal);
    mParser.add("--p-max-distance", paramDocF, params.maxDistance);
    mParser.add("--p-max-gaps-length", paramDocF, params.maxGapsLength);
    mParser.add("--p-min-motion", paramDocF, params.minMotion);

    // parse command-line
    mParser.parse(argc, argv);

    // if requested, display help and exit
    if (mHelp) {
        mParser.printHelp(std::cerr);
        std::exit(-1);
    }

    // if requested, display defaults and exit
    if (mDefaults) {
        mDefaults = false;
        mParser.printValues(std::cerr, '\n');
        std::exit(-1);
    }

    // if requested, display list and exit
    if (mList) {
        auto names = fmo::Algorithm::listFactories();
        for (auto& name : names) { std::cerr << name << '\n'; }
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
