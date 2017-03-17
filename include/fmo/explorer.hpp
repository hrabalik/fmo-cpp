#ifndef FMO_EXPLORER_HPP
#define FMO_EXPLORER_HPP

#include <fmo/algorithm.hpp>
#include <fmo/image.hpp>
#include <fmo/pointset.hpp>
#include <memory>

namespace fmo {
    /// Class for determining the presence of fast moving objects by analyzing a stream of images.
    struct Explorer {
        using Config = Algorithm::Config;
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

        /// Information about a fast-moving object.
        struct Object {
            Bounds bounds;   ///< location in the input image
            PointSet points; ///< point locations
            Image diff1;     ///< difference image of this frame and the previous (cropped)
            Image diff2;     ///< difference image of the previous frame (cropped)
            Image diffAnd;   ///< intersection of differences (cropped)
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
