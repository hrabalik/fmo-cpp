#include "args.hpp"

namespace {
    using doc_t = const char* const;
    doc_t inputDoc = "<path> Path to an input video file. Can be used multiple times. Must not be "
                     "used with --camera.";
    doc_t gtDoc = "<path> Text file containing ground truth data. Using this option enables "
                  "quality evaluation. If used at all, this option must be used as many times as "
                  "--file.";
    doc_t cameraDoc = "<int> Input camera device ID. When this option is used, stream from the "
                      "specified camera will be used as input. Using ID 0 selects the default "
                      "camera, if available. This option cannot be used with --input.";
    doc_t stopAtDoc = "<frame-num> Pause playback when the frame with the specified number is "
                      "encountered. When this option is used, --input option must be used exactly "
                      "once.";
    doc_t stopOnFpDoc = "Playback will stop whenever a detection is deemed a false positive. This "
                        "option can only be used when --gt is used.";
    doc_t stopOnFnDoc = "Playback will stop whenever a detection is deemed a false negative. This "
                        "option can only be used when --gt is used.";
    doc_t outDoc = "<path> File to save quality evaluation results to. This option can only be "
                   "used when --gt is used.";
    doc_t baselineDoc = "<path> File with previously saved quality evaluation results to compare "
                        "the results to. When used, the playback will stop to demonstrate where "
                        "the results differ. This option can only be used when --gt is used.";
}

Args::Args() {
    mParser.add("--input", inputDoc, [this](const std::string& path) { inputs.push_back(path); });
    mParser.add("--gt", gtDoc, [this](const std::string& path) { gts.push_back(path); });
    mParser.add("--camera", cameraDoc, [this](int id) { camera = id; });
    mParser.add("--stop-at", stopAtDoc, [this](int frame) { stopAt = frame; });
    mParser.add("--stop-on-fp", stopOnFpDoc, [this]() { stopOnFp = true; });
    mParser.add("--stop-on-fn", stopOnFnDoc, [this]() { stopOnFn = true; });
    mParser.add("--out", outDoc, [this](const std::string& path) { out = path; });
    mParser.add("--baseline", baselineDoc, [this](const std::string& path) { baseline = path; });
}
