#include "loop.hpp"
#include <algorithm>
#include <fmo/processing.hpp>
#include <fmo/region.hpp>

namespace {
    const fmo::PointSet emptySet;
}

DebugVisualizer::DebugVisualizer(Status& s) {
    s.window.setBottomLine("[space] pause | [enter] step | [,][.] jump 10 frames | [esc] quit");
}

void DebugVisualizer::visualize(Status& s, const fmo::Region&, const Evaluator* evaluator,
                                fmo::Algorithm& algorithm) {
    // draw the debug image provided by the algorithm
    fmo::copy(algorithm.getDebugImage(), mVis);
    s.window.print(s.inputName);
    s.window.print("frame: " + std::to_string(s.frameNum));

    // get details if there was an object
    const fmo::PointSet* points = &emptySet;
    if (algorithm.haveObject()) {
        algorithm.getObjectDetails(mDetailsCache);
        points = &mDetailsCache.points;
    }

    // draw detected points vs. ground truth
    if (evaluator != nullptr) {
        auto& result = evaluator->getResult();
        s.window.print(result.str());

        drawPointsGt(*points, evaluator->groundTruth(s.frameNum), mVis);
        s.window.setTextColor(good(result.eval) ? Colour::green() : Colour::red());
    } else {
        drawPoints(*points, mVis, Colour::lightMagenta());
    }

    // display
    s.window.display(mVis);

    // process keyboard input
    bool step = false;
    do {
        auto command = s.window.getCommand(s.paused);
        if (command == Command::PAUSE) s.paused = !s.paused;
        if (command == Command::STEP) step = true;
        if (command == Command::QUIT) s.quit = true;

        if (!s.haveCamera()) {
            if (command == Command::JUMP_BACKWARD) {
                s.paused = false;
                s.args.frame = std::max(1, s.frameNum - 10);
                s.reload = true;
            }
            if (command == Command::JUMP_FORWARD) {
                s.paused = false;
                s.args.frame = s.frameNum + 10;
            }
        }
    } while (s.paused && !step && !s.quit);
}

DemoVisualizer::DemoVisualizer(Status& s) { s.window.setBottomLine(""); }

void DemoVisualizer::visualize(Status& s, const fmo::Region& frame, const Evaluator*,
                               fmo::Algorithm&) {
    // draw input image as background
    fmo::copy(frame, mVis);

    // display
    s.window.display(mVis);

    // process keyboard input
    auto command = s.window.getCommand(false);
    if (command == Command::QUIT) s.quit = true;
}
