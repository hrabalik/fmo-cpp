#ifndef FMO_DESKTOP_LOOP_HPP
#define FMO_DESKTOP_LOOP_HPP

#include "args.hpp"
#include "evaluator.hpp"
#include "window.hpp"
#include <fmo/algorithm.hpp>
#include <fmo/stats.hpp>

struct Status {
    Args args;             ///< user settings
    Window window;         ///< GUI handle
    Results results;       ///< evaluation results
    Results baseline;      ///< previous evaluation results
    fmo::Timer timer;      ///< timer for the whole run
    std::string inputName; ///< name of the currently played back input
    int frameNum;          ///< frame number (first frame is frame 1)
    bool paused = false;   ///< playback paused
    bool quit = false;     ///< exit application now
    bool reload = false;   ///< load the same video again

    Status(int argc, char** argv) : args(argc, argv) {}
    bool haveCamera() const { return args.camera != -1; }
    bool haveWait() const { return args.wait != -1; }
    bool haveFrame() const { return args.frame != -1; }
    void unsetFrame() { args.frame = -1; }
};

void processVideo(Status& s, size_t inputNum);

struct Visualizer {
    virtual void visualize(Status& s, const Evaluator* evaluator, fmo::Algorithm& algorithm) = 0;
};

struct DebugVisualizer : public Visualizer {
    virtual void visualize(Status& s, const Evaluator* evaluator,
                           fmo::Algorithm& algorithm) override;

private:
    fmo::Image mVis;
    fmo::Algorithm::ObjectDetails mDetailsCache;
};

#endif // FMO_DESKTOP_LOOP_HPP
