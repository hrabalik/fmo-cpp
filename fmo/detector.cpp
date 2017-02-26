#include "fastext/FASTex.hpp"
#include <fmo/processing.hpp>
#include <fmo/detector.hpp>

namespace fmo {
    const size_t LEVELS = 6;
    const size_t SKIPPED_LEVELS = 1;
    const size_t PROCESSED_LEVELS = LEVELS - SKIPPED_LEVELS;

    /// Implementation details of class Detector.
    struct Detector::Impl {
        Impl() : fastText(19, true, cmp::FastFeatureDetectorC::KEY_POINTS_WHITE) {
            keypoints.resize(PROCESSED_LEVELS);
        }

        void setInput(const Mat& src) {
            fmo::pyramid(src, cascade, LEVELS);
            cv::Mat noMask;

            for (size_t i = 0; i < PROCESSED_LEVELS; i++) {
                Image& image = cascade[i];
                auto& imageKeypoints = keypoints[i + SKIPPED_LEVELS];
                fastText.detect(image.wrap(), imageKeypoints, noMask);
            }
        }

        const std::vector<Image>& getDebugImages() {
            return debugVis;
        }

    private:
        std::vector<Image> cascade;
        std::vector<Image> debugVis;
        cmp::FASTextGray fastText;
        std::vector<std::vector<cmp::FastKeyPoint>> keypoints;
    };

    Detector::~Detector() = default;
    Detector::Detector() = default;
    Detector::Detector(Detector&&) = default;
    Detector& Detector::operator=(Detector&&) = default;
    void Detector::setInput(const Mat& src) { mImpl->setInput(src); }
    const std::vector<Image>& Detector::getDebugImages() { return mImpl->getDebugImages(); }
}
