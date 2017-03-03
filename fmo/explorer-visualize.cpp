#include "explorer-impl.hpp"
#include "include-opencv.hpp"

namespace fmo {
    namespace {
        inline cv::Point toCvPoint(fmo::EPoint p) {
            return {int(std::round(p.x)), int(std::round(p.y))};
        }
    }

    void Explorer::Impl::visualize() {
        mVisualized.resize(Format::GRAY, mCfg.dims);
        cv::Mat mat = mVisualized.wrap();

        if (mIgnoredLevels.empty()) {
            mat.setTo(0);
        } else {
            // cover the visualization image with the highest-resolution difference image
            cv::resize(mIgnoredLevels[0].image.wrap(), mat,
                       cv::Size{mCfg.dims.width, mCfg.dims.height}, 0, 0, cv::INTER_NEAREST);
        }

        // draw keypoints
        auto kpIt = begin(mKeypoints);
        auto metaIt = begin(mKeypointsMeta);
        for (auto& level : mLevels) {
            int halfWidth = level.step / 2;
            for (int i = 0; i < level.numKeypoints; i++, kpIt++, metaIt++) {
                auto kp = *kpIt;
                int halfHeight = metaIt->halfHeight;
                cv::Point p1{int(kp.x) - halfWidth, int(kp.y) - halfHeight};
                cv::Point p2{int(kp.x) + halfWidth, int(kp.y) + halfHeight};
                cv::rectangle(mat, p1, p2, 0xFF);
            }
        }

        if (mHaveObject) {
            // draw keypoint line
            Line leftEdge{1, 0, 0};
            Line rightEdge{-1, 0, float(mCfg.dims.width)};
            EPoint l = euclidean(intersect(mKeypointLine, leftEdge));
            EPoint r = euclidean(intersect(mKeypointLine, rightEdge));
            cv::line(mat, toCvPoint(l), toCvPoint(r), 0xFF);
        }
    }
}
