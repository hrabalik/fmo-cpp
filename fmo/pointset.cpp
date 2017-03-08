#include <fmo/assert.hpp>
#include <fmo/pointset.hpp>
#include <fstream>
#include <stdexcept>

namespace fmo {
    void FrameSet::load(const std::string& filename) {
        std::ios::sync_with_stdio(false);

        auto fail = []() { throw std::runtime_error("failed to load point file"); };
        std::ifstream in{filename};
        if (!in) fail();
        int nonEmptyFrames;
        in >> mDims.width >> mDims.height >> mNumFrames >> nonEmptyFrames;
        if (!in) fail();

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
                frame.set.push_back({x, y});
            }

            if (!in) fail();

            std::sort(begin(frame.set), end(frame.set), pointSetComp);
        }

        FMO_ASSERT(std::is_sorted(begin(mFrames), end(mFrames),
                                  [](const Frame& l, const Frame& r) { return l.n < r.n; }),
                   "point file frames not sorted");
    }
}
