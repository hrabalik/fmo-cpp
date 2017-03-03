#ifndef FMO_EXPLORER_HPP
#define FMO_EXPLORER_HPP

#include <fmo/image.hpp>
#include <memory>

namespace fmo {
    /// Class for determining the presence of fast moving objects by analyzing a stream of images.
    struct Explorer {
        /// Configuration settings used by the constructor.
        struct Config {
            /// This value must be changed to match the input image size.
            Dims dims = {0, 0};
            /// Objects will be ignored unless they are at a distance from other objects in the
            /// image. This value is relative to image height.
            float minGap = 0.10;
            /// Maximum image height for processing. The input image will be downscaled by a factor
            /// of 2 until its height is less or equal to the specified value.
            int maxHeight = 300;
            /// Minimum image height for processing. The input image will be downscaled as long as
            /// its height is greater or equal to this value.
            int minHeight = 30;
        };

        Explorer(const Explorer&) = delete;
        Explorer& operator=(const Explorer&) = delete;

        ~Explorer();
        Explorer(const Config& cfg);
        Explorer(Explorer&&);
        Explorer& operator=(Explorer&&);

        /// Called every frame, providing the next image for processing. The processing will take
        /// place during the call and might take some time.
        void setInput(const Mat& src);

        /// Visualizes the result of detection, returning an image that should be displayed to the
        /// user.
        const Image& getDebugImage();

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl; ///< implementation details of class Explorer
    };
}

#endif // FMO_EXPLORER_HPP
