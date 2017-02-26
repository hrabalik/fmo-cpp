#ifndef FMO_DETECTOR_HPP
#define FMO_DETECTOR_HPP

#include <fmo/image.hpp>
#include <memory>

namespace fmo {
    /// Class for determining the presence of fast moving objects by analyzing the difference image
    /// of two consecutive frames.
    struct Detector {
        Detector(const Detector&) = delete;
        Detector& operator=(const Detector&) = delete;

        ~Detector();
        Detector();
        Detector(Detector&&);
        Detector& operator=(Detector&&);

        /// Detects fast moving objects in the provided difference image of two consecutive frames.
        /// The input image must be grayscale and must have both width and height divisible by 8.
        void setInput(const Mat& src);

        /// Visualizes the detected keypoints into a set of images (one for each scale).
        const std::vector<Image>& getDebugImages();

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}

#endif // FMO_DETECTOR_HPP
