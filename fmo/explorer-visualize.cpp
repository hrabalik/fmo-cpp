#include "explorer-impl.hpp"
#include "include-opencv.hpp"

namespace fmo {
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

        // draw strips
        auto kpIt = begin(mStrips);
        for (auto& level : mLevels) {
            int halfWidth = level.step / 2;
            for (int i = 0; i < level.numStrips; i++, kpIt++) {
                auto kp = *kpIt;
                cv::Point p1{kp.x - halfWidth, kp.y - kp.halfHeight};
                cv::Point p2{kp.x + halfWidth, kp.y + kp.halfHeight};
                cv::rectangle(mat, p1, p2, 0xFF);
            }
        }

        // draw components
        for (auto& comp : mComponents) {
            Strip* kp = &mStrips[comp.first];
            while (true) {
                if (kp->special == Strip::END) break;
                Strip* next = &mStrips[kp->special];
                cv::Point p1{kp->x, kp->y};
                cv::Point p2{next->x, next->y};
                cv::line(mat, p1, p2, 0xFF);
                kp = next;
            }
        }
    }
}
