#include "frameset.hpp"
#include "loop.hpp"
#include "video.hpp"
#include <fmo/processing.hpp>
#include <fmo/stats.hpp>

namespace {
    const fmo::PointSet emptySet;
}

void processVideo(Status& s, size_t inputNum) {
    // open input
    auto input = (!s.haveCamera()) ? VideoInput::makeFromFile(s.args.inputs.at(inputNum))
                                   : VideoInput::makeFromCamera(s.args.camera);
    auto dims = input->dims();
    float fps = input->fps();
    s.inputName = (!s.haveCamera()) ? extractFilename(s.args.inputs.at(inputNum))
                                    : "camera " + std::to_string(s.args.camera);

    // open GT
    std::unique_ptr<Evaluator> evaluator;
    if (!s.args.gts.empty()) {
        evaluator =
            std::make_unique<Evaluator>(s.args.gts.at(inputNum), dims, s.results, s.baseline);
    }

    // set speed
    if (!s.haveCamera()) {
        float waitSec = s.haveWait() ? (float(s.args.wait) / 1e3f) : (1.f / fps);
        s.window.setFrameTime(waitSec);
    }

    // setup caches
    fmo::Algorithm::Config explorerCfg{fmo::Algorithm::defaultName(), fmo::Format::BGR, dims};
    auto algorithm = fmo::Algorithm::make(explorerCfg);
    fmo::Image frameCopy{fmo::Format::BGR, dims};
    fmo::Algorithm::ObjectDetails detailsCache;

    for (s.frameNum = 1; !s.quit && !s.reload; s.frameNum++) {
        // workaround: linux waits for 5 sec when there's no more frames
        if (evaluator) {
            if (s.frameNum > evaluator->numFrames()) break;
        }

        // read video
        auto frame = input->receiveFrame();
        if (frame.data() == nullptr) break;

        // process
        fmo::copy(frame, frameCopy, fmo::Format::BGR);
        algorithm->setInputSwap(frameCopy);

        // get details if an object was detected
        const fmo::PointSet* points = &emptySet;
        if (algorithm->haveObject()) {
            algorithm->getObjectDetails(detailsCache);
            points = &detailsCache.points;
        }

        // evaluate
        if (evaluator) {
            auto result = evaluator->evaluateFrame(*points, s.frameNum);
            if (s.args.pauseFn && result.eval == Evaluation::FN) s.paused = true;
            if (s.args.pauseFp && result.eval == Evaluation::FP) s.paused = true;
            if (s.args.pauseRg && result.comp == Comparison::REGRESSION) s.paused = true;
            if (s.args.pauseIm && result.comp == Comparison::IMPROVEMENT) s.paused = true;
        }

        // pause when the sought-for frame number is encountered
        if (s.args.frame == s.frameNum) {
            s.unsetFrame();
            s.paused = true;
        }

        // skip other steps if seeking
        if (s.haveFrame()) continue;

        // skip other steps if in headless mode (but not paused)
        if (s.args.headless && !s.paused) continue;

        // visualize
        s.visualizer->visualize(s, frame, evaluator.get(), *algorithm);
    }
}
