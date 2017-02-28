#include <fmo/detector2.hpp>
#include <fmo/processing.hpp>
#include <opencv2/imgproc.hpp>

namespace fmo {
    /// Implementation details of class Detector2.
    struct Detector2::Impl {
        Impl(Config cfg) : mCfg(cfg) {
            if (mCfg.dims.width <= 0 || mCfg.dims.height <= 0) {
                throw std::runtime_error("bad config");
            }

            // create as many decimation levels as required to reach minimum height
            int step = 2;
            int width = mCfg.dims.width / 2;
            for (int height = mCfg.dims.height / 2; height >= mCfg.minHeight; height /= 2) {
                if (height > mCfg.maxHeight) {
                    mIgnoredLevels.emplace_back();
                    mIgnoredLevels.back().image.resize(Format::GRAY, {width, height});
                } else {
                    mLevels.emplace_back();
                    mLevels.back().image.resize(Format::GRAY, {width, height});
                    mLevels.back().step = step;
                    break; // make only one level
                }
                step *= 2;
                width /= 2;
            }
        }

        void setInput(const Mat& src) {
            createLevelPyramid(src);

            mKeypoints.clear();
            mKeypointsMeta.clear();
            for (auto& level : mLevels) {
                preprocess(level);
                findKeypoints(level);
            }
        }

        const Image& getDebugImage() {
            mDebugVis.resize(Format::GRAY, mCfg.dims);
            cv::Mat mat = mDebugVis.wrap();

            if (mIgnoredLevels.empty()) {
                mat = 0;
            } else {
                // cover the debug image with the highest-resolution difference image
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
                    cv::Point p1{int(kp[0]) - halfWidth, int(kp[1]) - halfHeight};
                    cv::Point p2{int(kp[0]) + halfWidth, int(kp[1]) + halfHeight};
                    cv::rectangle(mat, p1, p2, 0xFF);
                }
            }

            return mDebugVis;
        }

    private:
        struct IgnoredLevel {
            Image image;
        };

        struct Level {
            Image image;
            int step;
            int numKeypoints;
        };

        struct KeypointMeta {
            KeypointMeta(int aHalfHeight) : halfHeight(aHalfHeight) {}

            int halfHeight;
        };

        /// Create low-resolution versions of the source image using decimation.
        void createLevelPyramid(const Mat& src) {
            const Mat* prevLevelImage = &src;

            for (auto& level : mIgnoredLevels) {
                fmo::decimate(*prevLevelImage, level.image);
                prevLevelImage = &level.image;
            }

            for (auto& level : mLevels) {
                fmo::decimate(*prevLevelImage, level.image);
                prevLevelImage = &level.image;
            }
        }

        void preprocess(Level& level) {
            cv::Mat imageMat = level.image.wrap();
            cv::threshold(imageMat, imageMat, 19, 0xFF, cv::THRESH_BINARY);
        }

        void findKeypoints(Level& level) {
            level.numKeypoints = 0;
            Dims dims = level.image.dims();
            uint8_t* colData = level.image.data();

            int blackPrev = 0;
            int black = 0;
            int whitePrev = 0;
            int white = 0;
            int col = 0;
            int row = 0;
            int step = level.step;
            int halfStep = level.step / 2;
            int minGap = int(mCfg.minGap * dims.height);

            // Called after a black strip has ended (including at the end of the column). Creates a
            // keypoint if the previous two black and one white strip satisfy all conditions.
            auto check = [&, this]() {
                if (blackPrev >= minGap && black >= minGap && whitePrev > 0 && whitePrev >= 1) {
                    int x = (col * step) + halfStep;
                    int y = ((row - black) * step) - (whitePrev * halfStep);
                    mKeypoints.emplace_back(float(x), float(y));
                    mKeypointsMeta.emplace_back(whitePrev * halfStep);
                    level.numKeypoints++;
                }
            };

            for (col = 0; col < dims.width; col++, colData++) {
                uint8_t* data = colData;
                blackPrev = 0;
                black = minGap; // don't limit by top edge
                whitePrev = 0;
                white = 0;

                for (row = 0; row < dims.height; row++, data += dims.width) {
                    if (*data != 0) {
                        if (white++ == 0) {
                            check();
                            blackPrev = black;
                            black = 0;
                        }
                    } else {
                        if (black++ == 0) {
                            whitePrev = white;
                            white = 0;
                        }
                    }
                }
                black += minGap; // don't limit by bottom edge
                row += minGap;   // don't limit by bottom edge
                check();
            }
        }

        // data
        std::vector<IgnoredLevel> mIgnoredLevels;
        std::vector<Level> mLevels;
        std::vector<cv::Vec2f> mKeypoints;
        std::vector<KeypointMeta> mKeypointsMeta;
        Image mDebugVis;
        const Config mCfg;
    };

    Detector2::~Detector2() = default;
    Detector2::Detector2(Config cfg) : mImpl(new Impl(cfg)) {}
    Detector2::Detector2(Detector2&&) = default;
    Detector2& Detector2::operator=(Detector2&&) = default;
    void Detector2::setInput(const Mat& src) { mImpl->setInput(src); }
    const Image& Detector2::getDebugImage() { return mImpl->getDebugImage(); }
}
