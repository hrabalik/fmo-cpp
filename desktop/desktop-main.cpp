#include "args.hpp"
#include "evaluator.hpp"
#include "video.hpp"
#include "window.hpp"
#include <fmo/algebra.hpp>
#include <fmo/explorer.hpp>
#include <fmo/processing.hpp>
#include <iostream>

void loadGt(fmo::FrameSet& out, const std::string& filename, fmo::Dims dims) {
    try {
        out.load(filename);
        auto outDims = out.dims();
        if (outDims.width != dims.width || fmo::abs(outDims.height - dims.height) > 8) {
            throw std::runtime_error("video size inconsistent with ground truth");
        }
    } catch (std::exception& e) {
        std::cerr << "while loading file '" << filename << "'\n";
        throw e;
    }
}

int main(int argc, char** argv) try {
    Args args(argc, argv);
    if (args.help) return -1;

    bool fileMode = !args.inputs.empty();
    bool cameraMode = args.camera != -1;
    bool gtMode = !args.gts.empty();
    bool recordMode = !args.recordDir.empty();
    bool waitMode = args.wait != -1;

    auto videoInput = fileMode ? VideoInput::makeFromFile(args.inputs[0])
                               : VideoInput::makeFromCamera(args.camera);
    auto dims = videoInput->dims();
    float fps = videoInput->fps();

    fmo::FrameSet gt;
    if (gtMode) { loadGt(gt, args.gts[0], dims); }

    std::unique_ptr<VideoOutput> videoOutput;
    if (recordMode) { videoOutput = VideoOutput::makeInDirectory(args.recordDir, dims, fps); }

    Window window;

    if (!cameraMode) {
        float waitSec = waitMode ? (float(args.wait) / 1e3f) : (1.f / fps);
        window.setFrameTime(waitSec);
    }

    fmo::Explorer::Config explorerCfg;
    explorerCfg.dims = dims;
    fmo::Explorer explorer{explorerCfg};
    fmo::Image input{fmo::Format::GRAY, dims};
    fmo::Image vis{fmo::Format::BGR, dims};
    fmo::Explorer::Object object;
    Evaluator eval;

    bool paused = false;
    int frameNum = 1;
    for (bool quit = false; !quit; frameNum++) {
        // read and write video
        auto frame = videoInput->receiveFrame();
        if (frame.data() == nullptr) break;
        if (recordMode) { videoOutput->sendFrame(frame); }

        // process
        fmo::convert(frame, input, fmo::Format::GRAY);
        explorer.setInputSwap(input);

        // evaluate
        explorer.getObject(object);
        auto& gtPoints = gt.get(frameNum - 1);
        if (gtMode) {
            auto result = eval.eval(object.points, gtPoints);
            if (args.pauseOnFn && result == Evaluator::Result::FN) paused = true;
            if (args.pauseOnFp && result == Evaluator::Result::FP) paused = true;
        }

        // visualize
        fmo::copy(explorer.getDebugImage(), vis);
        if (gtMode) {
            drawPointsGt(object.points, gtPoints, vis);
        } else {
            drawPoints(object.points, vis, Color{0xFF, 0x00, 0x00});
        }
        window.display(vis);

        // process keyboard input
        bool step = false;
        do {
            auto command = window.getCommand(paused);
            if (command == Command::PAUSE) paused = !paused;
            if (command == Command::STEP) step = true;
            if (command == Command::QUIT) quit = true;
        } while (paused && !step && !quit);
    }

    // display results if there was a reference GT file
    if (gtMode) {
        std::cout << "TP: " << eval.count(Evaluator::Result::TP) << '\n';
        std::cout << "TN: " << eval.count(Evaluator::Result::TN) << '\n';
        std::cout << "FP: " << eval.count(Evaluator::Result::FP) << '\n';
        std::cout << "FN: " << eval.count(Evaluator::Result::FN) << '\n';
    }

} catch (std::exception& e) {
    std::cerr << "error: " << e.what() << '\n';
    std::cerr << "tip: use --help to see a list of available commands\n";
    return -1;
}
