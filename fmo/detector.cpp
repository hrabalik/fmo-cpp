#include "fastext/FASTex.hpp"
#include <fmo/detector.hpp>
#include <fmo/processing.hpp>

namespace fmo {
    const int THRESHOLD = 12;
    const bool NON_MAX_SUP = true;
    const size_t LEVELS = 6;
    const size_t SKIPPED_LEVELS = 1;
    const size_t PROCESSED_LEVELS = LEVELS - SKIPPED_LEVELS;

    /// Implementation details of class Detector.
    struct Detector::Impl {
        Impl() : fastText(THRESHOLD, NON_MAX_SUP, cmp::FastFeatureDetectorC::KEY_POINTS_WHITE) {
            keypoints.resize(PROCESSED_LEVELS);
            debugVis.resize(PROCESSED_LEVELS);
        }

        void setInput(const Mat& src) {
            fmo::pyramid(src, cascade, LEVELS);
            cv::Mat noMask;

            for (size_t i = 0; i < PROCESSED_LEVELS; i++) {
                Image& image = cascade[i + SKIPPED_LEVELS];
                auto& imageKeypoints = keypoints[i];
                fastText.detect(image.wrap(), imageKeypoints, noMask);
            }
        }

        const std::vector<Image>& getDebugImages() {
            for (size_t i = 0; i < PROCESSED_LEVELS; i++) {
                Image& image = cascade[i + SKIPPED_LEVELS];
                Image& out = debugVis[i];
                fmo::convert(image, out, Format::BGR);

                // draw keypoints
                cv::Mat outMat = out.wrap();
                for (auto& keypoint : keypoints[i]) {
                    cv::Point pt{static_cast<int>(keypoint.pt.x), static_cast<int>(keypoint.pt.y)};
                    outMat.at<cv::Vec3b>(pt) = cv::Vec3b(0, 0, 255);
                }
            }
            return debugVis;
        }

    private:
        std::vector<Image> cascade;
        std::vector<Image> debugVis;
        cmp::FASTextGray fastText;
        std::vector<std::vector<cmp::FastKeyPoint>> keypoints;
    };

    Detector::~Detector() = default;
    Detector::Detector() : mImpl(new Impl) {}
    Detector::Detector(Detector&&) = default;
    Detector& Detector::operator=(Detector&&) = default;
    void Detector::setInput(const Mat& src) { mImpl->setInput(src); }
    const std::vector<Image>& Detector::getDebugImages() { return mImpl->getDebugImages(); }
}
