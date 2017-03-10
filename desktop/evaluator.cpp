#include "evaluator.hpp"
#include "desktop-opencv.hpp"
#include <fmo/image.hpp>
#include <iostream>
#include <stdexcept>

namespace {
    const char* const dataStart = "###DATA_START###";
}

// Results

Results::File& Results::newFile(const std::string& name) {
    auto found = mMap.find(name);

    if (found != end(mMap)) {
        std::cerr << "duplicate result name '" << name << "'";
        throw std::runtime_error("duplicate result name");
    }

    mList.emplace_front();
    mMap.emplace(name, &mList.front());
    return mList.front();
}

const Results::File& Results::getFile(const std::string& name) const {
    auto found = mMap.find(name);

    if (found == end(mMap)) {
        static File empty;
        return empty;
    }

    return *found->second;
}



// Evaluator

Evaluator::Evaluator(Results& results, const std::string& gtFilename, fmo::Dims dims) {
    mGt.load(gtFilename, dims);

    {
        // strip the path off the filename before using it as name
        auto findLast = [&](char c) {
            auto re = rend(gtFilename);
            auto f = std::find(rbegin(gtFilename), re, c);
            if (f == re) return 0;
            return int(&*f - gtFilename.data()) + 1;
        };
        int skip = std::max(findLast('\\'), findLast('/'));
        mName.assign(begin(gtFilename) + skip, end(gtFilename));
    }

    mResults = &results.newFile(mName);
    mResults->reserve(mGt.numFrames());
}

Evaluation Evaluator::evaluateFrame(const fmo::PointSet& ps, int frameNum) {
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

    if (ps.empty()) {
        if (gt.empty()) {
            mResults->push_back(Evaluation::TN);
        } else {
            mResults->push_back(Evaluation::FN);
        }
    } else {
        double iou = double(intersection) / double(union_);
        if (iou > IOU_THRESHOLD) {
            mResults->push_back(Evaluation::TP);
        } else {
            mResults->push_back(Evaluation::FP);
        }
    }

    return mResults->back();
}
