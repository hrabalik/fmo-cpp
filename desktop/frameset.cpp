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
    in >> mDims.width >> mDims.height >> allFrames >> nonEmptyFrames;
    if (!in) fail();

    if (mDims.width != dims.width || fmo::abs(mDims.height - dims.height) > 8) {
        throw std::runtime_error("dimensions inconsistent with video");
    }

    mFrames.resize(allFrames);
    for (int i = 0; i < nonEmptyFrames; i++) {
        int frameNum, numPoints;
        in >> frameNum >> numPoints;
        if (!in) fail();

        if (frameNum < 1 || frameNum > allFrames) {
            std::cerr << "got frame number " << frameNum << ", want <= " << allFrames << '\n';
            throw std::runtime_error("bad frame number");
        }

        auto& ptr = at(frameNum);
        ptr = std::make_unique<fmo::PointSet>();
        auto& set = *ptr;

        for (int j = 0; j < numPoints; j++) {
            int index;
            in >> index;
            int x = (index - 1) / mDims.height;
            int y = (index - 1) % mDims.height;
            if (y < dims.height) { set.push_back({x, y}); }
        }

        if (!in) fail();
        std::sort(begin(set), end(set), fmo::pointSetComp);
    }
} catch (std::exception& e) {
    std::cerr << "while loading file '" << filename << "'\n";
    throw e;
}

const fmo::PointSet& FrameSet::get(int frameNum) const {
    static const fmo::PointSet empty;
    if (frameNum < 1 || frameNum > numFrames()) return empty;
    auto& ptr = at(frameNum);
    if (!ptr) return empty;
    return *ptr;
}
