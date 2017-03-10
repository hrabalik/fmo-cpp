#include <fmo/algebra.hpp>
#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fmo {
    void FrameSet::load(const std::string& filename, fmo::Dims dims) try {
        std::ios::sync_with_stdio(false);
        mFrames.clear();

        auto fail = []() { throw std::runtime_error("failed to parse file"); };
        std::ifstream in{filename};
        if (!in) fail();
        int nonEmptyFrames;
        in >> mDims.width >> mDims.height >> mNumFrames >> nonEmptyFrames;
        if (!in) fail();

        if (mDims.width != dims.width || fmo::abs(mDims.height - dims.height) > 8) {
            throw std::runtime_error("dimensions inconsistent with video");
        }

        for (int i = 0; i < nonEmptyFrames; i++) {
            mFrames.emplace_back();
            auto& frame = mFrames.back();
            int numPoints;
            in >> frame.n >> numPoints;

            for (int j = 0; j < numPoints; j++) {
                int index;
                in >> index;
                int x = (index - 1) / mDims.height;
                int y = (index - 1) % mDims.height;
                if (y < dims.height) { frame.set.push_back({x, y}); }
            }

            if (!in) fail();

            std::sort(begin(frame.set), end(frame.set), pointSetComp);
        }

        if (!std::is_sorted(begin(mFrames), end(mFrames),
                            [](const Frame& l, const Frame& r) { return l.n < r.n; })) {
            throw std::runtime_error("frames not sorted in point file");
        }
    } catch (std::exception& e) {
        std::cerr << "while loading file '" << filename << "'\n";
        throw e;
    }
}
