#ifndef FMO_EXPLORER_HPP
#define FMO_EXPLORER_HPP

#include <fmo/image.hpp>
#include <fmo/pointset.hpp>
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
            /// Minimum distance that an object must travel in a single frame. This value is
            /// relative to the length of the path travelled in three frames.
            float minMotion = 0.25;
        };

        Explorer(const Explorer&) = delete;
        Explorer& operator=(const Explorer&) = delete;

        ~Explorer();
        Explorer(const Config& cfg);
        Explorer(Explorer&&);
        Explorer& operator=(Explorer&&);

        /// Called every frame, providing the next image for processing. The processing will take
        /// place during the call and might take some time. The input is received by swapping the
        /// contents of the provided input image with an internal buffer.
        void setInputSwap(Image& input);

        /// Visualizes the result of detection, returning an image that should be displayed to the
        /// user.
        const Image& getDebugImage();

        /// A rectangular area.
        struct Bounds {
            Pos min; ///< minimum coordinates of the box that encloses the object
            Pos max; ///< maximum coordinates of the box that encloses the object
        };

        /// Information about a fast-moving object.
        struct Object {
            Bounds bounds;   ///< location in the input image
            PointSet points; ///< point locations
        };

        /// Determines whether a new object has been found as a result of analyzing the last frame.
        bool haveObject() const;

        /// Provides information about the object that has been just found. Use haveObject() to
        /// check whether it makes sense to call this method.
        void getObject(Object& out) const;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl; ///< implementation details of class Explorer
    };
}

#endif // FMO_EXPLORER_HPP
