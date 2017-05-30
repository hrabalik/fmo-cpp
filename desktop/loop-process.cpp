#include "loop.hpp"
#include "objectset.hpp"
#include "video.hpp"
#include <fmo/processing.hpp>
#include <fmo/stats.hpp>

namespace {
    const std::vector<fmo::PointSet> noObjects;
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

    // write sequence
    std::unique_ptr<DetectionReport::Sequence> sequenceReport;
    if (s.rpt) { sequenceReport = s.rpt->makeSequence(s.inputName); }

    // set speed
    if (!s.haveCamera()) {
        float waitSec = s.haveWait() ? (float(s.args.wait) / 1e3f) : (1.f / fps);
        s.window.setFrameTime(waitSec);
    }

    // setup caches
    fmo::Format format = s.args.yuv ? fmo::Format::YUV : fmo::Format::BGR;
    std::vector<fmo::PointSet> objectVec;
    objectVec.resize(1);
    auto algorithm = fmo::Algorithm::make(s.args.params, format, dims);
    fmo::Region frame;
    fmo::Image frameCopy{format, dims};
    fmo::Algorithm::Output outputCache;
    EvalResult evalResult;
    s.inFrameNum = 1;
    s.outFrameNum = 1 + algorithm->getOutputOffset();

    for (; !s.quit && !s.reload; s.inFrameNum++, s.outFrameNum++) {
        // end the video early when GT requests it
        bool allowNewFrames = true;
        if (evaluator) {
            int numGtFrames = evaluator->gt().numFrames();
            if (s.outFrameNum > numGtFrames) {
                // end the loop once evaluation is done
                break;
            } else if (s.inFrameNum > numGtFrames) {
                // stop receiving fresh frames once GT ends
                allowNewFrames = false;
            }
        }

        // read video
        if (allowNewFrames) {
            frame = input->receiveFrame();
            if (frame.data() == nullptr) {
                // end the loop unconditionally when a new frame is needed but is not available
                break;
            }
        }

        // process
        fmo::convert(frame, frameCopy, format);
        algorithm->setInputSwap(frameCopy);
        algorithm->getOutput(outputCache);

        // evaluate
        if (evaluator) {
            if (s.outFrameNum >= 1) {
                evaluator->evaluateFrame(outputCache, s.outFrameNum, evalResult, s.args.params.iouThreshold);
                if (s.args.pauseFn && evalResult.eval[Event::FN] > 0) s.paused = true;
                if (s.args.pauseFp && evalResult.eval[Event::FP] > 0) s.paused = true;
                if (s.args.pauseRg && evalResult.comp == Comparison::REGRESSION) s.paused = true;
                if (s.args.pauseIm && evalResult.comp == Comparison::IMPROVEMENT) s.paused = true;
            } else {
                evalResult.clear();
                evalResult.comp = Comparison::BUFFERING;
            }
        }

        // write to detection report
        if (sequenceReport) { sequenceReport->writeFrame(s.outFrameNum, outputCache, evalResult); }

        // pause when the sought-for frame number is encountered
        if (s.args.frame == s.inFrameNum) {
            s.unsetFrame();
            s.paused = true;
        }

        // skip other steps if seeking
        if (s.haveFrame()) continue;

        // skip visualization if in headless mode (but not paused)
        if (s.args.headless && !s.paused) continue;

        // visualize
        s.visualizer->visualize(s, frame, evaluator.get(), evalResult, *algorithm);
    }
}
