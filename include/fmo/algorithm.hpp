#ifndef FMO_ALGORITHM_HPP
#define FMO_ALGORITHM_HPP

#include <fmo/image.hpp>
#include <fmo/pointset.hpp>
#include <functional>
#include <memory>
#include <string>

namespace fmo {
    /// Represents a detection algorithm. Specific algorithms are implemented in different
    /// compilation units as subclasses of this class, and then registered using the
    /// registerFactory() static method. To create a new instance of an algorithm, call the make()
    /// static method.
    struct Algorithm {
        Algorithm() = default;
        Algorithm(const Algorithm&) = delete;
        Algorithm& operator=(const Algorithm&) = delete;
        virtual ~Algorithm() {}
        struct Config;

        /// Type of a function that produces an Algorithm instance.
        using Factory = std::function<std::unique_ptr<Algorithm>(const Config&)>;

        /// Configuration settings determining the properties of a new Algorithm instance. Pass the
        /// object into the make() static method.
        struct Config {
            /// Name of the algorithm.
            const std::string name;
            /// Input image format.
            const Format format;
            /// Input image size.
            const Dims dims;
            /// Objects will be ignored unless they are at a distance from other objects in the
            /// image. This value is relative to image height.
            float minGap = 0.10;
            /// Maximum image height for processing. The input image will be downscaled by a factor
            /// of 2 until its height is less or equal to the specified value.
            int maxHeight = 300;
            /// Minimum distance that an object must travel in a single frame. This value is
            /// relative to the length of the path travelled in three frames.
            float minMotion = 0.25;
            /// When outputting object point set, specifies what resolution should be used. When
            /// using source resolution, additional heavy-weight calculations need to be performed.
            enum { PROCESSING, SOURCE } objectResolution = PROCESSING;

            /// Creates a new config instance, forcing the user to fill in the mandatory values.
            Config(std::string aName, Format aFormat, Dims aDims)
                : name(std::move(aName)), format(aFormat), dims(aDims) {}

            /// Formats an Image in such a way that it is viable for use as an input image of the
            /// algorithm.
            void resizeAsInput(Image& image) const { image.resize(format, dims); }
        };

        /// A structure that contains all relevant information about a detected object. The caller
        /// is expected to cache this object and use it with the getObjectDetails() method. The
        /// object contains auxiliary images that may be used by the getObjectDetails() method in
        /// the process of generating object information.
        struct ObjectDetails {
            Bounds bounds;   ///< location in the input image
            PointSet points; ///< point locations
            Image temp1;     ///< object for storing auxiliary image data
            Image temp2;     ///< object for storing auxiliary image data
            Image temp3;     ///< object for storing auxiliary image data
        };

        /// Creates a new instance of an Algorithm. The field config.name is used to determine which
        /// algorithm factory will be used. The factory must have been previously added with the
        /// registerFactory() static method.
        static std::unique_ptr<Algorithm> make(const Config& config);

        /// Registers an algorithm factory so that it is available via a call to make(). The
        /// provided name will be used as the search key and must be unique.
        static void registerFactory(const std::string& name, const Factory& factory);

        /// To be called every frame, providing the next image for processing. The processing will
        /// take place during the call and might take a long time. The input is received by swapping
        /// the contents of the provided input image with an internal buffer.
        virtual void setInputSwap(Image& input) = 0;

        /// Visualizes the result of detection, returning an image that is useful for debugging
        /// algorithm behavior. The returned image will have BGR format and the same dimensions as
        /// the input image.
        virtual const Image& getDebugImage() = 0;

        /// Determines whether a new object has been found as a result of analyzing the last frame
        /// during a call to setInputSwap(). When this method returns true, the methods
        /// getObjectBounds() and getObjectDetails() may be called to get more information about the
        /// detected object.
        virtual bool haveObject() const = 0;

        /// Provides the bounding box that encloses the detected object. Use the haveObject() method
        /// first to check if an object has been detected in this frame.
        virtual Bounds getObjectBounds() const = 0;

        /// Provides detailed information about the detected object, including a list of object
        /// pixels. Use the haveObject() method first to check if an object has been detected in
        /// this frame.
        virtual void getObjectDetails(ObjectDetails& details) const = 0;
    };
}

#endif // FMO_ALGORITHM_HPP
