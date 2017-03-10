#include "args.hpp"
#include "evaluator.hpp"
#include "frameset.hpp"
#include "video.hpp"
#include "window.hpp"
#include <fmo/explorer.hpp>
#include <fmo/processing.hpp>
#include <iostream>

struct Status {
    Args args;           ///< user settings
    Window window;       ///< GUI handle
    bool paused = false; ///< playback paused
    bool quit = false;   ///< exit application now

    Status(int argc, char** argv) : args(argc, argv) {}
};

void processVideo(Status& s, size_t inputNum);

int main(int argc, char** argv) try {
    Status s{argc, argv};

    if (s.args.camera != -1) { s.args.inputs.emplace_back("camera " + s.args.camera); }

    for (size_t i = 0; i < s.args.inputs.size(); i++) {
        try {
            processVideo(s, i);
        } catch (std::exception& e) {
            std::cerr << "while playing '" << s.args.inputs.at(i) << "'\n";
            throw e;
        }
    }
} catch (std::exception& e) {
    std::cerr << "error: " << e.what() << '\n';
    std::cerr << "tip: use --help to see a list of available commands\n";
    return -1;
}

void processVideo(Status& s, size_t inputNum) {
    // open input
    auto input = (s.args.camera == -1) ? VideoInput::makeFromFile(s.args.inputs.at(inputNum))
                                       : VideoInput::makeFromCamera(s.args.camera);
    auto dims = input->dims();
    float fps = input->fps();

    // open GT
    std::unique_ptr<Evaluator> evaluator;
    if (!s.args.gts.empty()) {
        evaluator = std::make_unique<Evaluator>(s.args.gts.at(inputNum), dims);
    }

    // open output
    std::unique_ptr<VideoOutput> output;
    if (!s.args.recordDir.empty()) {
        output = VideoOutput::makeInDirectory(s.args.recordDir, dims, fps);
    }

    // set speed
    if (s.args.camera == -1) {
        float waitSec = (s.args.wait != -1) ? (float(s.args.wait) / 1e3f) : (1.f / fps);
        s.window.setFrameTime(waitSec);
    }

    // setup caches
    fmo::Explorer::Config explorerCfg;
    explorerCfg.dims = dims;
    fmo::Explorer explorer{explorerCfg};
    fmo::Image gray{fmo::Format::GRAY, dims};
    fmo::Image vis{fmo::Format::BGR, dims};
    fmo::Explorer::Object object;

    for (int frameNum = 1; !s.quit; frameNum++) {
        // read and write video
        auto frame = input->receiveFrame();
        if (frame.data() == nullptr) break;
        if (output) { output->sendFrame(frame); }

        // process
        fmo::convert(frame, gray, fmo::Format::GRAY);
        explorer.setInputSwap(gray);

        // evaluate
        explorer.getObject(object);
        if (evaluator) {
            auto result = evaluator->evaluateFrame(object.points, frameNum);
            if (s.args.pauseOnFn && result == Evaluation::FN) s.paused = true;
            if (s.args.pauseOnFp && result == Evaluation::FP) s.paused = true;
        }

        // visualize
        fmo::copy(explorer.getDebugImage(), vis);
        if (evaluator) {
            drawPointsGt(object.points, evaluator->groundTruth(frameNum), vis);
        } else {
            drawPoints(object.points, vis, Color{0xFF, 0x00, 0x00});
        }
        s.window.display(vis);

        // process keyboard input
        bool step = false;
        do {
            auto command = s.window.getCommand(s.paused);
            if (command == Command::PAUSE) s.paused = !s.paused;
            if (command == Command::STEP) step = true;
            if (command == Command::QUIT) s.quit = true;
        } while (s.paused && !step && !s.quit);
    }
}
