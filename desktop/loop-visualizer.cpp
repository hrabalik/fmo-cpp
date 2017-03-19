#include "loop.hpp"
#include "recorder.hpp"
#include <algorithm>
#include <fmo/processing.hpp>
#include <fmo/region.hpp>

namespace {
    const fmo::PointSet emptySet;
}

DebugVisualizer::DebugVisualizer(Status& s) {
    s.window.setBottomLine("[esc] quit | [space] pause | [enter] step | [,][.] jump 10 frames");
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

DemoVisualizer::DemoVisualizer(Status& s) { updateHelp(s); }

void DemoVisualizer::updateHelp(Status& s) {
    if (!mShowHelp) {
        s.window.setBottomLine("");
    } else {
        if (mAutomatic) {
            s.window.setBottomLine("[esc] quit | [m] switch to manual mode");
        } else {
            s.window.setBottomLine(
                "[esc] quit | [a] switch to automatic mode | [r] start/stop recording");
        }
    }
}

void DemoVisualizer::printStatus(Status& s) const {
    bool recording;

    if (mAutomatic) {
        s.window.print("automatic mode");
        recording = mAutomatic->isRecording();
    } else {
        s.window.print("manual mode");
        recording = bool(mManual);
    }

    s.window.print(recording ? "recording" : "not recording");
    s.window.setTextColor(recording ? Colour::lightRed() : Colour::lightGray());

    s.window.print("[?] for help");
}

void DemoVisualizer::visualize(Status& s, const fmo::Region& frame, const Evaluator*,
                               fmo::Algorithm&) {
    // record frames
    if (mAutomatic) {
        bool event = mForcedEvent;
        mAutomatic->frame(frame, event);
    } else if (mManual) {
        mManual->frame(frame);
    }
    mForcedEvent = false;

    // draw input image as background
    fmo::copy(frame, mVis);

    // display
    printStatus(s);
    s.window.display(mVis);

    // process keyboard input
    auto command = s.window.getCommand(false);
    if (command == Command::QUIT) { s.quit = true; }
    if (command == Command::SHOW_HELP) {
        mShowHelp = !mShowHelp;
        updateHelp(s);
    }
    if (command == Command::AUTOMATIC_MODE) {
        if (mManual) { mManual.reset(nullptr); }
        if (!mAutomatic) {
            mAutomatic = std::make_unique<AutomaticRecorder>(s.args.recordDir, frame.format(),
                                                             frame.dims(), 30.f);
            updateHelp(s);
        }
    }
    if (command == Command::FORCED_EVENT) { mForcedEvent = true; }
    if (command == Command::MANUAL_MODE) {
        if (mAutomatic) {
            mAutomatic.reset(nullptr);
            updateHelp(s);
        }
    }
    if (command == Command::RECORD) {
        if (mManual) {
            mManual.reset(nullptr);
        } else if (!mAutomatic) {
            mManual = std::make_unique<ManualRecorder>(s.args.recordDir, frame.format(),
                                                       frame.dims(), 30.f);
        }
    }
}
