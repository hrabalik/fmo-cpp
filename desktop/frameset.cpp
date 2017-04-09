#include "frameset.hpp"
#include <fmo/algebra.hpp>
#include <fmo/assert.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

void FrameSet::load(const std::string& filename, fmo::Dims dims) try {
    std::ios::sync_with_stdio(false);
    mFrames.clear();

    auto fail = []() { throw std::runtime_error("failed to parse file"); };
    std::ifstream in{filename};
    if (!in) fail();
    int allFrames, nonEmptyFrames;
    in >> mDims.width >> mDims.height >> allFrames >> mOffset >> nonEmptyFrames;
    if (!in) fail();

    if (mDims.width != dims.width || std::abs(mDims.height - dims.height) > 8) {
        throw std::runtime_error("dimensions inconsistent with video");
    }

    auto addPoints =
        [ npt = dims.width * dims.height, dims ](fmo::PointSet & set, int first, int last) {
        last = std::min(last, npt);
        for (; first < last; first++) {
            int x = first % dims.width;
            int y = first / dims.width;
            set.push_back({x, y});
        }
    };

    mFrames.resize(allFrames);
    for (int i = 0; i < nonEmptyFrames; i++) {
        int frameNum, numRuns;
        in >> frameNum >> numRuns;
        if (!in) fail();

        if (frameNum < 1 || frameNum > allFrames) {
            std::cerr << "got frame number " << frameNum << ", want <= " << allFrames << '\n';
            throw std::runtime_error("bad frame number");
        }

        auto& ptr = at(frameNum);
        ptr = std::make_unique<fmo::PointSet>();
        auto& set = *ptr;

        bool white = false;
        int pos = 0;
        for (int j = 0; j < numRuns; j++) {
            int runLength;
            in >> runLength;
            if (white) { addPoints(set, pos, pos + runLength); }
            pos += runLength;
            white = !white;
        }

        if (white) { addPoints(set, pos, mDims.width * mDims.height); }

        if (!in) fail();
        std::sort(begin(set), end(set), fmo::pointSetComp);
    }
} catch (std::exception& e) {
    std::cerr << "while loading file '" << filename << "'\n";
    throw e;
}

const fmo::PointSet& FrameSet::get(int frameNum) const {
    static const fmo::PointSet empty;
    frameNum += mOffset;
    if (frameNum < 1 || frameNum > numFrames()) return empty;
    auto& ptr = at(frameNum);
    if (!ptr) return empty;
    return *ptr;
}
