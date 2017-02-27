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
            for (int height = mCfg.dims.height; height >= mCfg.minHeight; height /= 2) {
                if (height > mCfg.maxHeight) {
                    mIgnoredLevels.emplace_back();
                    mIgnoredLevels.back().image.resize(Format::GRAY, {width, height});
                } else {
                    mLevels.emplace_back();
                    mLevels.back().image.resize(Format::GRAY, {width, height});
                    mLevels.back().step = step;
                }
                step *= 2;
                width /= 2;
            }
        }

        void setInput(const Mat& src) {
            createLevelPyramid(src);

            for (auto& level : mLevels) { findKeypoints(level); }
        }

        const Image& getDebugImage() { return mDebugVis; }

    private:
        struct IgnoredLevel {
            Image image;
        };

        struct Level {
            Image image;
            int step;
        };

        struct Keypoint {
            Keypoint(int aX, int aY) : x(aX), y(aY) {}
            int x, y;
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

        void findKeypoints(Level& level) {
            cv::Mat imageMat = level.image.wrap();
            cv::threshold(imageMat, imageMat, 19, 0xFF, cv::THRESH_BINARY);

            Dims dims = level.image.dims();
            uint8_t* colData = level.image.data();

            int blackPrev = 0;
            int black = 0;
            int whitePrev = 0;
            int white = 0;
            int col = 0;
            int row = 0;

            auto check = [&]() {
                if (blackPrev >= 10 && black >= 10 && whitePrev > 0 && whitePrev <= 3) {
                    int x = row - black - (whitePrev / 2);
                    int y = col;
                    int step = level.step;
                    int offset = level.step / 2;
                    x = (x * step) + offset;
                    y = (y * step) + offset;
                    this->mKeypoints.emplace_back(x, y);
                }
            };

            for (col = 0; col < dims.width; col++, colData++) {
                uint8_t* data = colData;
                blackPrev = 0;
                black = 0;
                whitePrev = 0;
                white = 0;

                for (row = 0; row < dims.height; row++, data += dims.width) {
                    if (*data == 0) {
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
                check();
            }
        }

        std::vector<IgnoredLevel> mIgnoredLevels;
        std::vector<Level> mLevels;
        std::vector<Keypoint> mKeypoints;
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
