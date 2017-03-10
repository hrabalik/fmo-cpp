#include "evaluator.hpp"
#include "desktop-opencv.hpp"
#include <fmo/image.hpp>
#include <iostream>
#include <stdexcept>

Evaluator::~Evaluator() {
    // todo update aggregator
}

Evaluator::Evaluator(const std::string& gtFilename, fmo::Dims dims) {
    mGt.load(gtFilename, dims);
    mResults.reserve(mGt.numFrames());

    {
        // strip the path off the filename before storing it
        auto findLast = [&](char c) {
            auto re = rend(gtFilename);
            auto f = std::find(rbegin(gtFilename), re, c);
            if (f == re) return 0;
            return int(&*f - gtFilename.data()) + 1;
        };
        int skip = std::max(findLast('\\'), findLast('/'));
        mGtName.assign(begin(gtFilename) + skip, end(gtFilename));
    }
}

Result Evaluator::evaluateFrame(const fmo::PointSet& ps, int frameNum) {
    if (++mFrameNum != frameNum) {
        std::cerr << "got frame: " << frameNum << " expected: " << mFrameNum << '\n';
        throw std::runtime_error("unexpected frame number");
    }

    int intersection = 0;
    int union_ = 0;

    auto mismatch = [&](fmo::Pos) { union_++; };
    auto match = [&](fmo::Pos) {
        intersection++;
        union_++;
    };

    auto& gt = groundTruth(mFrameNum);
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

    return mResults.back();
}
