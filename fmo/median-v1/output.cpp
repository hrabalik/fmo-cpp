#include "algorithm-median.hpp"

namespace fmo {
    bool MedianV1::haveObject() const {
        for (auto& object : outputObjects()) {
            if (object.selected) return true;
        }
        return false;
    }

    void MedianV1::getObjectDetails(ObjectDetails& out) const {
        //
        return;
    }
}
