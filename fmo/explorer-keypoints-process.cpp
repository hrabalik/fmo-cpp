#include "explorer-impl.hpp"
#include "include-opencv.hpp"

namespace fmo {
    void Explorer::Impl::processKeypoints() {
        // reset success status
        mHaveObject = false;
    }
}
