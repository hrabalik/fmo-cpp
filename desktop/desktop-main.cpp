#include "args.hpp"
#include "evaluator.hpp"
#include "frameset.hpp"
#include "video.hpp"
#include "window.hpp"
#include <fmo/explorer.hpp>
#include <fmo/processing.hpp>
#include <iostream>

struct Status {
    Args args;                         ///< user settings
    Window window;                     ///< GUI handle
    std::unique_ptr<VideoInput> input; ///< video reader
    std::unique_ptr<FrameSet> gt;      ///< ground truth
    bool paused = false;               ///< playback paused
    bool quit = false;                 ///< exit application now

    Status(int argc, char** argv) : args(argc, argv) {}
};

void processVideo(Status& s);

int main(int argc, char** argv) try {
    Status s{argc, argv};

    if (s.args.camera != -1) {
        s.input = VideoInput::makeFromCamera(s.args.camera);
        try {
            processVideo(s);
        } catch (std::exception& e) {
            std::cerr << "while streaming from camera " << s.args.camera << "\n";
            throw e;
        }
    } else {
        for (int i = 0; !s.quit && i < int(s.args.inputs.size()); i++) {
            s.input = VideoInput::makeFromFile(s.args.inputs[i]);

            try {
                if (!s.args.gts.empty()) {
                    if (!s.gt) s.gt = std::make_unique<FrameSet>();
                    s.gt->load(s.args.gts[i], s.input->dims());
                }

                processVideo(s);
            } catch (std::exception& e) {
                std::cerr << "while playing video '" << s.args.inputs[i] << "'\n";
                throw e;
            }
        }
    }
} catch (std::exception& e) {
    std::cerr << "error: " << e.what() << '\n';
    std::cerr << "tip: use --help to see a list of available commands\n";
    return -1;
}

void processVideo(Status& s) {
    if (s.args.camera == -1) {
        float waitSec = (s.args.wait != -1) ? (float(s.args.wait) / 1e3f) : (1.f / s.input->fps());
        s.window.setFrameTime(waitSec);
    }

    std::unique_ptr<VideoOutput> output;
    if (!s.args.recordDir.empty()) {
        output = VideoOutput::makeInDirectory(s.args.recordDir, s.input->dims(), s.input->fps());
    }

    fmo::Explorer::Config explorerCfg;
    explorerCfg.dims = s.input->dims();
    fmo::Explorer explorer{explorerCfg};
    fmo::Image input{fmo::Format::GRAY, s.input->dims()};
    fmo::Image vis{fmo::Format::BGR, s.input->dims()};
    fmo::Explorer::Object object;
    Evaluator eval;

    for (int frameNum = 1; !s.quit; frameNum++) {
        // read and write video
        auto frame = s.input->receiveFrame();
        if (frame.data() == nullptr) break;
        if (output) { output->sendFrame(frame); }

        // process
        fmo::convert(frame, input, fmo::Format::GRAY);
        explorer.setInputSwap(input);

        // evaluate
        explorer.getObject(object);
        const fmo::PointSet* gtPoints = nullptr;
        if (s.gt) {
            gtPoints = &s.gt->get(frameNum - 1);
            auto result = eval.eval(object.points, *gtPoints);
            if (s.args.pauseOnFn && result == Evaluator::Result::FN) s.paused = true;
            if (s.args.pauseOnFp && result == Evaluator::Result::FP) s.paused = true;
        }

        // visualize
        fmo::copy(explorer.getDebugImage(), vis);
        if (s.gt) {
            drawPointsGt(object.points, *gtPoints, vis);
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
