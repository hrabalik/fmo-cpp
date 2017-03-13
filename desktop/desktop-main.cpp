#include "args.hpp"
#include "evaluator.hpp"
#include "frameset.hpp"
#include "report.hpp"
#include "video.hpp"
#include "window.hpp"
#include <fmo/explorer.hpp>
#include <fmo/processing.hpp>
#include <fmo/stats.hpp>
#include <iostream>

struct Status {
    Args args;           ///< user settings
    Window window;       ///< GUI handle
    Results results;     ///< evaluation results
    Results baseline;    ///< previous evaluation results
    fmo::Timer timer;    ///< timer for the whole run
    bool paused = false; ///< playback paused
    bool quit = false;   ///< exit application now
    bool reload = false; ///< laod the same video again

    Status(int argc, char** argv) : args(argc, argv) {}
};

void processVideo(Status& s, size_t inputNum);

int main(int argc, char** argv) try {
    Status s{argc, argv};

    if (!s.args.baseline.empty()) { s.baseline.load(s.args.baseline); }
    if (s.args.camera != -1) { s.args.inputs.emplace_back(); }

    for (size_t i = 0; !s.quit && i < s.args.inputs.size(); i++) {
        try {
            do {
                s.reload = false;
                processVideo(s, i);
            } while (s.reload);
        } catch (std::exception& e) {
            std::cerr << "while playing '" << s.args.inputs.at(i) << "'\n";
            throw e;
        }
    }

    Report report(s.results, s.baseline, s.timer.toc<fmo::TimeUnit::SEC, float>());
    report.write(std::cout);
    if (!s.args.evalDir.empty()) { report.save(s.args.evalDir); }
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
    std::string inputName = (s.args.camera == -1) ? extractFilename(s.args.inputs.at(inputNum))
                                                  : "camera " + std::to_string(s.args.camera);

    // open GT
    std::unique_ptr<Evaluator> evaluator;
    if (!s.args.gts.empty()) {
        evaluator =
            std::make_unique<Evaluator>(s.args.gts.at(inputNum), dims, s.results, s.baseline);
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

    for (int frameNum = 1; !s.quit && !s.reload; frameNum++) {
        // workaround: linux waits for 5 sec when there's no more frames
        if (evaluator) {
            if (frameNum > evaluator->numFrames()) break;
        }

        // read and write video
        auto frame = input->receiveFrame();
        if (frame.data() == nullptr) break;
        if (output) { output->sendFrame(frame); }

        // process
        fmo::convert(frame, gray, fmo::Format::GRAY);
        explorer.setInputSwap(gray);

        // evaluate
        explorer.getObject(object);
        EvalResult result;
        if (evaluator) {
            result = evaluator->evaluateFrame(object.points, frameNum);
            if (s.args.pauseFn && result.eval == Evaluation::FN) s.paused = true;
            if (s.args.pauseFp && result.eval == Evaluation::FP) s.paused = true;
            if (s.args.pauseRg && result.comp == Comparison::REGRESSION) s.paused = true;
            if (s.args.pauseIm && result.comp == Comparison::IMPROVEMENT) s.paused = true;
        }

        // pause when the sought-for frame number is encountered
        if (s.args.frame == frameNum) {
            s.args.frame = -1;
            s.paused = true;
        }

        // skip other steps if in headless mode or if seeking
        if ((s.args.frame != -1 || s.args.headless) && !s.paused) continue;

        // visualize
        fmo::copy(explorer.getDebugImage(), vis);
        s.window.print(inputName);
        s.window.print("frame: " + std::to_string(frameNum));
        if (evaluator) {
            s.window.print(result.str());
            drawPointsGt(object.points, evaluator->groundTruth(frameNum), vis);
            s.window.setTextColor(good(result.eval) ? Color{0x40, 0x80, 0x40}
                                                    : Color{0x40, 0x40, 0x80});
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

            if (s.args.camera == -1) {
                if (command == Command::JUMP_BACKWARD) {
                    s.paused = false;
                    s.args.frame = std::max(1, frameNum - 10);
                    s.reload = true;
                }
                if (command == Command::JUMP_FORWARD) {
                    s.paused = false;
                    s.args.frame = frameNum + 10;
                }
            }
        } while (s.paused && !step && !s.quit);
    }
}
