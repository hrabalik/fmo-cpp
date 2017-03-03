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

        //// draw components
        // for (auto& comp : mComponents) {
        //    Strip* strip = &mStrips[comp.first];
        //
        //    // connect strips in the component with lines
        //    while (true) {
        //        if (strip->special == Strip::END) break;
        //        Strip* next = &mStrips[strip->special];
        //        cv::Point p1{strip->x, strip->y};
        //        cv::Point p2{next->x, next->y};
        //        cv::line(mat, p1, p2, 0xFF);
        //        strip = next;
        //    }
        //
        //    // draw approximate half-height
        //    std::string text = std::to_string(comp.approxHalfHeight);
        //    cv::Point textOrigin{strip->x, strip->y};
        //    int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
        //    cv::putText(mat, text, textOrigin, fontFace, 0.5, 0xFF, 2);
        //}

        // draw trajectories
        for (auto& traj : mTrajectories) {
            Component* comp = &mComponents[traj.first];

            // connect components in the trajectory with lines
            while (comp->next != Component::NO_COMPONENT) {
                Component* next = &mComponents[comp->next];
                Strip& s1 = mStrips[comp->last];
                Strip& s2 = mStrips[next->first];
                cv::Point p1{s1.x, s1.y};
                cv::Point p2{s2.x, s2.y};
                cv::line(mat, p1, p2, 0xFF);
                comp = next;
            }
        }
    }
}
