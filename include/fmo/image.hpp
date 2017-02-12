#ifndef FMO_IMAGE_HPP
#define FMO_IMAGE_HPP

#include <cstdint>
#include <memory>
#include <string>

namespace cv {
    class Mat;
}

namespace fmo {
    /// An image buffer class. Wraps the OpenCV Mat class. Has value semantics, i.e. copying an
    /// instance of Image will perform a copy of the entire image data.
    struct alignas(8) Image {
        /// Possible internal color formats.
        enum class Format : char {
            UNKNOWN = 0,
            GRAY,
            BGR,
            YUV420SP,
        };

        /// Image dimensions.
        struct Size {
            int width, height;
        };

        Image();

        ~Image();

        /// Reads an image from file and converts it to the desired format.
        Image(const std::string& filename, Format format);

        /// Completely copies the argument to create a new Image.
        Image(const Image&);

        /// Completely copies the image on the right hand side to the left hand side.
        Image& operator=(const Image&);

        /// Moves image data from the argument without copying.
        Image(Image&&);

        /// Moves image data to the left hand side without copying.
        Image& operator=(Image&&);

        /// Provides current image dimensions.
        Size size() const { return mSize; }

        /// Provides current image format.
        Format format() const { return mFormat; }

        /// Provides direct access to the underlying data.
        uint8_t* data();

        /// Provides direct access to the underlying data.
        const uint8_t* data() const;

        /// Converts the image "src" to a given color format and saves the result to "dest".
        friend void convert(const Image& src, Image& dest, Format format);

    private:
        cv::Mat& mat();
        const cv::Mat& mat() const;

        // data
        char mHidden[(14 * 8) - sizeof(Format) - sizeof(Size)];
        Format mFormat = Format::UNKNOWN;
        Size mSize = {0, 0};
    };
}

#endif // FMO_IMAGE_HPP
