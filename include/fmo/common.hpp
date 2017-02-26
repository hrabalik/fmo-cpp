#ifndef FMO_COMMON_HPP
#define FMO_COMMON_HPP

#include <cstdint>

namespace cv {
    class Mat;
}

namespace fmo {
    // forward declarations
    struct Image;
    struct Region;

    /// Possible image color formats.
    enum class Format {
        UNKNOWN = 0,
        GRAY,
        BGR,
        INT32,
        YUV420SP,
    };

    /// Image location.
    struct Pos {
        int x, y;

        friend bool operator==(const Pos& lhs, const Pos& rhs) {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }

        friend bool operator!=(const Pos& lhs, const Pos& rhs) { return !(lhs == rhs); }
    };

    /// Image dimensions.
    struct Dims {
        int width, height;

        friend bool operator==(const Dims& lhs, const Dims& rhs) {
            return lhs.width == rhs.width && lhs.height == rhs.height;
        }

        friend bool operator!=(const Dims& lhs, const Dims& rhs) { return !(lhs == rhs); }
    };

    /// An object that represents the OpenCV Mat class. Use the wrap() method to create an instance
    /// of cv::Mat.
    struct Mat {
        ~Mat() = default;
        Mat() = default;
        Mat(const Mat&) = default;
        Mat& operator=(const Mat&) = default;
        Mat(Format format, Dims dims) : mFormat(format), mDims(dims) { }

        /// Provides current image format.
        Format format() const { return mFormat; }

        /// Provides current image dimensions.
        Dims dims() const { return mDims; }

        /// Creates a region that refers to a rectangular area inside the image.
        virtual Region region(Pos pos, Dims dims) = 0;

        /// The number of bytes to advance if one needs to access the next row.
        virtual size_t skip() const = 0;

        /// Provides access to image data.
        virtual uint8_t* data() = 0;

        /// Provides access to image data.
        virtual const uint8_t* data() const = 0;

        /// Provides access to the part of image data where UVis stored. Only for YUV420SP images.
        virtual uint8_t* uvData() = 0;

        /// Provides access to the part of image data where UVis stored. Only for YUV420SP images.
        virtual const uint8_t* uvData() const = 0;

        /// Resizes the image to match the desired format and dimensions.
        virtual void resize(Format format, Dims dims) = 0;

        /// Wraps the data pointer in a Mat object.
        virtual cv::Mat wrap() = 0;

        /// Wraps the data pointer in a Mat object. Be careful with this one -- use the returned Mat
        /// only for reading.
        virtual cv::Mat wrap() const = 0;

    protected:
        Format mFormat = Format::UNKNOWN;
        Dims mDims = {0, 0};
    };
}

#endif // FMO_COMMON_HPP
