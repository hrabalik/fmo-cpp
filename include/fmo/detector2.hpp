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
            /// This value must be changed to match the input image size.
            Dims dims = {0, 0};
            /// Maximum image height for processing. The input image will be downscaled by a factor
            /// of 2 until its height is less or equal to the specified value.
            int maxHeight = 300;
            /// Minimum image height for processing. The input image will be downscaled as long as
            /// its height is greater or equal to this value.
            int minHeight = 30;
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
        const Image& getDebugImage();

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}

#endif // FMO_DETECTOR2_HPP
