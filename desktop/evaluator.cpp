#include "evaluator.hpp"
#include "desktop-opencv.hpp"
#include <fmo/image.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

// EvalResult

std::string EvalResult::str() const {
    std::string result;
    result.reserve(80);
    result += "evaluated: ";
    if (eval == Evaluation::TP) result += "true positive ";
    if (eval == Evaluation::TN) result += "true negative ";
    if (eval == Evaluation::FP) result += "false positive ";
    if (eval == Evaluation::FN) result += "false negative ";
    if (comp == Comparison::IMPROVEMENT) result += "(improvement) ";
    if (comp == Comparison::REGRESSION) result += "(regression) ";
    if (comp == Comparison::SAME) result += "(no change) ";
    if (comp == Comparison::NONE) result += "(no baseline) ";
    return result;
}

// Results

Results::File& Results::newFile(const std::string& name) {
    auto found = mMap.find(name);

    if (found != mMap.end()) {
        std::cerr << "duplicate result name '" << name << "'";
        throw std::runtime_error("duplicate result name");
    }

    mList.emplace_front();
    mMap.emplace(name, &mList.front());
    return mList.front();
}

const Results::File& Results::getFile(const std::string& name) const {
    auto found = mMap.find(name);

    if (found == mMap.end()) {
        static File empty;
        return empty;
    }

    return *found->second;
}

void Results::load(const std::string& fn) try {
    mMap.clear();
    mList.clear();
    std::ifstream in{fn, std::ios_base::in | std::ios_base::binary};

    if (!in) { throw std::runtime_error("failed to open file"); }

    std::string token;
    bool found = false;
    while (!found && in >> token) { found = token == "###DATA_START###"; }
    if (!found) { throw std::runtime_error("failed to find data start token"); }

    int numFiles;
    in >> numFiles;

    for (int i = 0; i < numFiles; i++) {
        in >> std::quoted(token);
        auto& file = newFile(token);
        size_t numFrames;
        in >> numFrames;
        file.reserve(numFrames);
        in >> token;

        if (token.size() != numFrames) {
            std::cerr << "found " << token.size() << " frames, " << numFrames << " expected\n";
            throw std::runtime_error("bad frame count");
        }

        for (char c : token) { file.push_back(Evaluation(c - '0')); }
        if (!in) { throw std::runtime_error("error while parsing"); }
    }
} catch (std::exception& e) {
    std::cerr << "while reading '" << fn << "'\n";
    throw e;
}

// Evaluator

Evaluator::~Evaluator() {
    // if ended prematurely, remove the results
    if (mGt.numFrames() != mResults->size()) { mResults->clear(); }
}

Evaluator::Evaluator(const std::string& gtFilename, fmo::Dims dims, Results& results,
                     const Results& baseline) {
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

    mBaseline = &baseline.getFile(mName);
    if (mBaseline->empty()) mBaseline = nullptr;
    if (mBaseline != nullptr  && mBaseline->size() != mGt.numFrames()) {
        std::cerr << "baseline has " << mBaseline->size() << " frames, expecting "
                  << mGt.numFrames() << '\n';
        throw std::runtime_error("bad baseline number of frames");
    }
}

EvalResult Evaluator::evaluateFrame(const fmo::PointSet& ps, int frameNum) {
    if (++mFrameNum != frameNum) {
        std::cerr << "got frame: " << frameNum << " expected: " << mFrameNum << '\n';
        throw std::runtime_error("bad number of frames");
    }

    if (mFrameNum > mGt.numFrames()) {
        std::cerr << "GT has " << mGt.numFrames() << " frames, now at " << mFrameNum << '\n';
        throw std::runtime_error("movie length inconsistent with GT");
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

    Evaluation eval;
    if (ps.empty()) {
        if (gt.empty()) {
            eval = Evaluation::TN;
        } else {
            eval = Evaluation::FN;
        }
    } else {
        double iou = double(intersection) / double(union_);
        if (iou > IOU_THRESHOLD) {
            eval = Evaluation::TP;
        } else {
            eval = Evaluation::FP;
        }
    }
    mResults->push_back(eval);

    if (mBaseline != nullptr) {
        size_t idx = mResults->size() - 1;
        auto baseline = mBaseline->at(idx);
        bool goodBefore = good(baseline);
        bool goodNow = good(eval);
        if (!goodBefore && goodNow) return {eval, Comparison::IMPROVEMENT};
        if (goodBefore && !goodNow) return {eval, Comparison::REGRESSION};
        return {eval, Comparison::SAME};
    }

    return {eval, Comparison::NONE};
}
