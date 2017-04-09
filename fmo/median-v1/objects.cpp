#include "algorithm-median.hpp"

namespace fmo {
    void MedianV1::findObjects() {
        // reset
        mObjects[2].swap(mObjects[1]);
        mObjects[1].swap(mObjects[0]);
        mObjects[0].clear();

        // find interesting components
        for (auto& comp : mComponents) {
            int numStrips = 0;
            int16_t firstStrip = comp.first;
            int16_t lastStrip = firstStrip;

            for (int16_t i = firstStrip; i != Special::END; i = mNextStrip[i]) {
                numStrips++;
                lastStrip = i;
            }

            if (numStrips < mCfg.minStripsInObject) {
                comp.status = Component::TOO_FEW_STRIPS;
                continue;
            }

            // gather points
            mCache.lower.clear();
            mCache.upper.clear();
            for (int16_t i = firstStrip; i != Special::END; i = mNextStrip[i]) {
                auto& strip = mStrips[i];
                int16_t x1 = strip.pos.x - strip.halfDims.width;
                int16_t x2 = strip.pos.x + strip.halfDims.width;
                int16_t y1 = strip.pos.y - strip.halfDims.height;
                int16_t y2 = strip.pos.y + strip.halfDims.height;
                mCache.lower.emplace_back(x1, y1);
                mCache.lower.emplace_back(x2, y1);
                mCache.upper.emplace_back(x1, y2);
                mCache.upper.emplace_back(x2, y2);
            }

            // compute convex hull
            using pred_t = bool(const Vector&, const Vector&);
            auto hull = [](const std::vector<Pos16>& src, std::vector<Pos16>& dst, pred_t pred) {
                dst.clear();
                for (auto& pos : src) {
                    dst.push_back(pos);
                    size_t sz;
                    while ((sz = dst.size()) >= 3 &&
                           !pred(dst[sz - 2] - dst[sz - 3], dst[sz - 1] - dst[sz - 2])) {
                        dst[sz - 2] = dst[sz - 1];
                        dst.pop_back();
                    }
                }
            };

            hull(mCache.lower, mCache.temp, fmo::left);
            mCache.temp.swap(mCache.lower);
            hull(mCache.upper, mCache.temp, fmo::right);
            mCache.temp.swap(mCache.upper);

            // no problems encountered: add object
            comp.status = Component::GOOD;
        }
    }
}
