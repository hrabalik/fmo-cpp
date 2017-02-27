#ifndef FMO_DETECTOR2_HPP
#define FMO_DETECTOR2_HPP

#include <fmo/image.hpp>
#include <memory>

namespace fmo {
    /// Class for determining the presence of fast moving objects by analyzing the difference image
    /// of two consecutive frames.
    struct Detector2 {
        /// Configuration settings used by the constructor.
        struct Config {
            // the number of decimations applied to each input image
            int levels = 6;
            // the number of decimations that are not used for detection
            int skippedLevels = 1;
            // the minimal difference in gray levels required for two pixels to be considered
            // lighter or darker
            int threshold = 12;
            // apply non-maximal supression
            bool nonMaxSup = true;
            // TODO find out what this is
            int kMin = 9;
            // TODO find out what this is
            int kMax = 11;
            // apply thresholding before detection
            bool threshBeforeDetect = false;
        };

        Detector2(const Detector2&) = delete;
        Detector2& operator=(const Detector2&) = delete;

        ~Detector2();
        Detector2(Config cfg = Config());
        Detector2(Detector2&&);
        Detector2& operator=(Detector2&&);

        /// Detects fast moving objects in the provided difference image of two consecutive frames.
        /// The input image must be grayscale.
        void setInput(const Mat& src);

        /// Visualizes the detected keypoints into a set of images (one for each scale).
        const std::vector<Image>& getDebugImages();

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}

#endif // FMO_DETECTOR2_HPP
