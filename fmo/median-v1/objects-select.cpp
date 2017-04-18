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

            // compute the expected location of o2 assuming linear motion
            Vector motion = o1.center - o0.center;
            Pos expected = {o1.center.x + motion.x, o1.center.y + motion.y};

            float error = length(expected - o2.center) / (o0.halfLen[0] + o1.halfLen[0]);
            if (error > mCfg.selectMaxError) {
                // the third object is too far from the expected location
                continue;
            }

            // everything is fine: mark objects as selected
            o0.selected = true;
            o1.selected = true;
            o2.selected = true;
        }
    }
}
