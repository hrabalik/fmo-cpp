#include "algorithm-median.hpp"

namespace fmo {
    void MedianV1::selectObjects() {
        for (auto& o0 : mObjects[0]) {
            if (o0.prev == Special::END) {
                // no matched object in the previous frame
                continue;
            }
            auto& o1 = mObjects[1][o0.prev];
            if (o1.prev == Special::END) {
                // no matched object in the frame before the previous frame
                continue;
            }
            auto& o2 = mObjects[2][o1.prev];

            // ignore the triplet if it is not deemed a fast-moving object detection
            if (!selectable(o0, o1, o2)) continue;

            // everything is fine: mark objects as selected
            o0.selected = true;
            o1.selected = true;
            o2.selected = true;
        }
    }

    bool MedianV1::selectable(Object& o0, Object& o1, Object& o2) const {
        Vector v1 = o1.center - o0.center;
        Vector v2 = o2.center - o1.center;
        if (dot(v1, v2) < 0) { return false; }
        float d1, d2;
        NormVector nv1{v1, d1};
        NormVector nv2{v2, d2};
        float distance = std::max(d1, d2) / std::min(d1, d2);
        if (distance > mCfg.selectMaxDistance) { return false; }
        float curvature = std::abs(cross(nv1, nv2));
        if (curvature > mCfg.selectMaxCurvature) { return false; }
        return true;
    }
}
