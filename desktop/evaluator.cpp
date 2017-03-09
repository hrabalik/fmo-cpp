#include "evaluator.hpp"
#include "desktop-opencv.hpp"
#include <fmo/image.hpp>

void Evaluator::eval(const fmo::PointSet& ps, const fmo::PointSet& gt) {
    int intersection = 0;
    int union_ = 0;

    auto mismatch = [&](fmo::Pos) { union_++; };

    auto match = [&](fmo::Pos) {
        intersection++;
        union_++;
    };

    fmo::pointSetCompare(ps, gt, mismatch, mismatch, match);

    auto call = [this](Result r) {
        mCount[int(r)]++;
        mResults.push_back(r);
    };

    if (ps.empty()) {
        if (gt.empty()) {
            call(Result::TN);
        } else {
            call(Result::FN);
        }
    } else {
        double iou = double(intersection) / double(union_);
        if (iou > IOU_THRESHOLD) {
            call(Result::TP);
        } else {
            call(Result::FP);
        }
    }
}
