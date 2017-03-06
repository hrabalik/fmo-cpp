#include "explorer-impl.hpp"
#include "include-opencv.hpp"

namespace fmo {
    namespace {
        inline cv::Point toCv(Pos p) { return {p.x, p.y}; }
    }

    void Explorer::Impl::visualize() {
        mVisualized.resize(Format::GRAY, mCfg.dims);
        cv::Mat mat = mVisualized.wrap();

        if (!mIgnoredLevels.empty()) {
            // cover the visualization image with the highest-resolution difference image
            cv::resize(mIgnoredLevels[0].image.wrap(), mat,
                       cv::Size{mCfg.dims.width, mCfg.dims.height}, 0, 0, cv::INTER_NEAREST);
        }
        else {
            cv::resize(mLevel.image1.wrap(), mat, cv::Size{ mCfg.dims.width, mCfg.dims.height },
                       0, 0, cv::INTER_NEAREST);
        }

        // draw strips
        auto kpIt = begin(mStrips);
        {
            auto& level = mLevel;
            int halfWidth = level.step / 2;
            for (int i = 0; i < level.numStrips; i++, kpIt++) {
                auto kp = *kpIt;
                cv::Point p1{kp.x - halfWidth, kp.y - kp.halfHeight};
                cv::Point p2{kp.x + halfWidth, kp.y + kp.halfHeight};
                cv::rectangle(mat, p1, p2, 0xFF);
            }
        }

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

        // draw rejected objects
        for (auto* traj : mRejected) {
            auto bounds = findBounds(*traj);
            cv::rectangle(mat, toCv(bounds.min), toCv(bounds.max), 0x00);
        }

        // draw accepted objects
        for (auto* traj : mObjects) {
            auto bounds = findBounds(*traj);
            cv::rectangle(mat, toCv(bounds.min), toCv(bounds.max), 0xFF);
        }
    }
}
