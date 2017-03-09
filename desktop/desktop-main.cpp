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

    bool haveInput = !args.inputs.empty();
    bool haveCamera = args.camera != -1;
    bool haveGt = !args.gts.empty();
    bool haveRecordDir = !args.recordDir.empty();
    bool haveWait = args.wait != -1;

    auto videoInput = haveInput ? VideoInput::makeFromFile(args.inputs[0])
                                : VideoInput::makeFromCamera(args.camera);
    auto dims = videoInput->dims();
    float fps = videoInput->fps();

    fmo::FrameSet gt;
    if (haveGt) { loadGt(gt, args.gts[0], dims); }

    std::unique_ptr<VideoOutput> videoOutput;
    if (haveRecordDir) { videoOutput = VideoOutput::makeInDirectory(args.recordDir, dims, fps); }

    Window window;
    bool paused = false;
    bool step = false;
    bool quit = false;
    int frameNum = 0;

    if (!haveCamera) {
        float waitSec = haveWait ? (float(args.wait) / 1e3f) : (1.f / fps);
        window.setFrameTime(waitSec);
    }

    fmo::Explorer::Config explorerCfg;
    explorerCfg.dims = dims;
    fmo::Explorer explorer{explorerCfg};
    fmo::Image input{fmo::Format::GRAY, dims};
    fmo::Image vis{fmo::Format::BGR, dims};
    fmo::Explorer::Object object;
    Evaluator eval;

    auto processCommand = [&paused, &step, &quit](Command command) {
        switch (command) {
        case Command::PAUSE:
            paused = !paused;
            break;
        case Command::STEP:
            step = true;
            break;
        case Command::QUIT:
            quit = true;
            break;
        default:
            break;
        }
    };

    while (!quit) {
        if (step || !paused || frameNum == 0) {
            step = false;
            frameNum++;

            // read
            auto frame = videoInput->receiveFrame();
            if (frame.data() == nullptr) break;

            // write
            if (haveRecordDir) { videoOutput->sendFrame(frame); }

            // process
            fmo::convert(frame, input, fmo::Format::GRAY);
            explorer.setInputSwap(input);

            // visualize
            explorer.getObject(object);
            fmo::copy(explorer.getDebugImage(), vis);

            if (haveGt) {
                // with GT: evaluate
                eval.eval(object.points, gt.get(frameNum - 1), vis);
            } else {
                // without GT: draw ball in blue
                drawPoints(object.points, vis, Color{0xFF, 0x00, 0x00});
            }

            window.display(vis);
        }

        processCommand(window.getCommand(paused));
    }

    // display results if there was a reference GT file
    if (haveGt) {
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
