#include "explorer-impl.hpp"
#include "include-opencv.hpp"

namespace fmo {
    void Explorer::Impl::processKeypoints() {
        // reset success status
        mHaveObject = false;

        if (mKeypoints.size() < MIN_KEYPOINTS) return;
        cv::Vec4f line;
        cv::fitLine(mKeypoints, line, CV_DIST_L2, 0, 0.01, 0.01);

        // discard if line is close to vertical (exceeds 60 degrees, i.e. dx < 0.5)
        if (std::abs(line[0]) < 0.5) return;

        // convert line to projective coords -- {-dy, dx, dy*px - dx*py}
        mKeypointLine = {-line[1], line[0], line[1] * line[2] - line[0] * line[3]};

        // set sucess status
        mHaveObject = true;
    }
}
