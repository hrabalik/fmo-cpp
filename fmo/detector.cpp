#include "fastext/FASTex.hpp"
#include <fmo/detector.hpp>
#include <fmo/processing.hpp>
#include <opencv2/imgproc.hpp>

namespace fmo {
    /// Implementation details of class Detector.
    struct Detector::Impl {
        Impl(Config cfg)
            : mFASText(cfg.threshold, cfg.nonMaxSup, cmp::FastFeatureDetectorC::KEY_POINTS_WHITE,
                       cfg.kMin, cfg.kMax),
              mCfg(cfg),
              mProcessedLevels(cfg.levels - cfg.skippedLevels) {
            if (mCfg.levels < 0 || mCfg.skippedLevels < 0 || mProcessedLevels < 0) {
                throw std::runtime_error("Detector: bad config");
            }

            mKeypoints.resize(mProcessedLevels);
            mDebugVis.resize(mProcessedLevels);
        }

        void setInput(const Mat& src) {
            fmo::pyramid(src, mPyramid, mCfg.levels);
            cv::Mat noMask;

            for (size_t i = 0; i < mProcessedLevels; i++) {
                Image& input = mPyramid[i + mCfg.skippedLevels];
                auto& keypoints = mKeypoints[i];
                cv::Mat inputMat = input.wrap();

                if (mCfg.threshBeforeDetect) {
                    cv::threshold(inputMat, inputMat, 19, 0xFF, cv::THRESH_BINARY);
                }

                mFASText.detect(inputMat, keypoints, noMask);
            }
        }

        const std::vector<Image>& getDebugImages() {
            for (size_t i = 0; i < mProcessedLevels; i++) {
                Image& image = mPyramid[i + mCfg.skippedLevels];
                Image& out = mDebugVis[i];
                fmo::convert(image, out, Format::BGR);

                // draw keypoints
                cv::Mat outMat = out.wrap();
                for (auto& keypoint : mKeypoints[i]) {
                    cv::Point pt{static_cast<int>(keypoint.pt.x), static_cast<int>(keypoint.pt.y)};
                    outMat.at<cv::Vec3b>(pt) = cv::Vec3b(0, 0, 255);
                }
            }
            return mDebugVis;
        }

    private:
        std::vector<Image> mPyramid;
        std::vector<Image> mDebugVis;
        cmp::FASTextGray mFASText;
        std::vector<std::vector<cmp::FastKeyPoint>> mKeypoints;
        const Config mCfg;
        const int mProcessedLevels;
    };

    Detector::~Detector() = default;
    Detector::Detector(Config cfg) : mImpl(new Impl(cfg)) {}
    Detector::Detector(Detector&&) = default;
    Detector& Detector::operator=(Detector&&) = default;
    void Detector::setInput(const Mat& src) { mImpl->setInput(src); }
    const std::vector<Image>& Detector::getDebugImages() { return mImpl->getDebugImages(); }
}
