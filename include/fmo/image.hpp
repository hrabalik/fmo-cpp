#ifndef FMO_IMAGE_HPP
#define FMO_IMAGE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace cv {
    class Mat;
}

namespace fmo {
    /// An image buffer class. Wraps the OpenCV Mat class. Has value semantics, i.e. copying an
    /// instance of Image will perform a copy of the entire image data.
    struct Image {
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

        Image() = default;
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        /// Reads an image from file and converts it to the desired format.
        Image(const std::string& filename, Format format);

        /// Provides current image dimensions.
        Size size() const { return mSize; }

        /// Provides current image format.
        Format format() const { return mFormat; }

        /// Provides direct access to the underlying data.
        uint8_t* data() { return mData.data(); }

        /// Provides direct access to the underlying data.
        const uint8_t* data() const { return mData.data(); }

        // /// Converts the image "src" to a given color format and saves the result to "dest". The
        // /// images "src" and "dest" must not be the same object.
        // static void convert(const Image& src, Image& dest, Format format);

    private:
        /// Wraps the data pointer in a Mat object, after ensuring that the underlying array is
        /// large enough to hold image data of the required size.
        cv::Mat resize(Format format, Size size);

        /// Wraps the data pointer in a Mat object. Be careful with this one -- use the returned Mat
        /// only for reading.
        cv::Mat wrap() const;

        // data
        std::vector<uint8_t> mData;
        Format mFormat = Format::UNKNOWN;
        Size mSize = {0, 0};
    };
}

#endif // FMO_IMAGE_HPP
