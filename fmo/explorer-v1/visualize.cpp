#include "../include-opencv.hpp"
#include "explorer.hpp"
#include <fmo/processing.hpp>

namespace fmo {
    namespace {
        inline cv::Point toCv(Pos p) { return {p.x, p.y}; }
        const cv::Scalar stripsColor{0xC0, 0x00, 0x00};
        const cv::Scalar trajectoriesColor{0xC0, 0x00, 0x00};
        const cv::Scalar rejectedColor{0x80, 0x80, 0x80};
        const cv::Scalar acceptedColor{0xC0, 0x00, 0x00};
    }

    void ExplorerV1::visualize() {
        // cover the visualization image with the latest input image
        convert(mSourceLevel.image1, mCache.visColor, Format::BGR);
        cv::Mat result = mCache.visColor.wrap();

        // draw strips
        auto kpIt = begin(mStrips);
        {
            auto& level = mLevel;
            int halfWidth = level.step / 2;
            for (int i = 0; i < level.numStrips; i++, kpIt++) {
                auto kp = *kpIt;
                cv::Point p1{kp.x - halfWidth, kp.y - kp.halfHeight};
                cv::Point p2{kp.x + halfWidth, kp.y + kp.halfHeight};
                cv::rectangle(result, p1, p2, stripsColor);
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
                cv::line(result, p1, p2, stripsColor);
                comp = next;
            }
        }

        // draw rejected objects
        for (auto* traj : mRejected) {
            auto bounds = findBounds(*traj);
            cv::rectangle(result, toCv(bounds.min), toCv(bounds.max), rejectedColor);
        }

        // draw accepted objects
        for (auto* traj : mObjects) {
            auto bounds = findBounds(*traj);
            cv::rectangle(result, toCv(bounds.min), toCv(bounds.max), acceptedColor);
        }
    }
}
