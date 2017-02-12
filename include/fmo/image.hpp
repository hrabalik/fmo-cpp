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
        struct Dims {
            int width, height;

            friend bool operator==(const Dims& lhs, const Dims& rhs) {
                return lhs.width == rhs.width && lhs.height == rhs.height;
            }

            friend bool operator!=(const Dims& lhs, const Dims& rhs) { return !(lhs == rhs); }
        };

        using iterator = uint8_t*;
        using const_iterator = const uint8_t*;

        ~Image() = default;
        Image() = default;
        Image(const Image&) = default;
        Image& operator=(const Image&) = default;
        Image(Image&&);
        Image& operator=(Image&&);

        /// Reads an image from file and converts it to the desired format.
        Image(const std::string& filename, Format format);

        /// Copies an image from memory.
        Image(Format format, Dims dims, const uint8_t* data);

        /// Removes all data and sets the size to zero. Does not deallocate any memory.
        void clear();

        /// Provides current image dimensions.
        Dims dims() const { return mDims; }

        /// Provides current image format.
        Format format() const { return mFormat; }

        /// The number of bytes in the image.
        size_t size() const { return mData.size(); }

        /// Provides direct access to the underlying data.
        uint8_t* data() { return mData.data(); }

        /// Provides direct access to the underlying data.
        const uint8_t* data() const { return mData.data(); }

        /// Provides iterator access to the underlying data.
        iterator begin() { return mData.data(); }

        /// Provides iterator access to the underlying data.
        const_iterator begin() const { return mData.data(); }

        /// Provides iterator access to the underlying data.
        friend iterator begin(Image& img) { return img.mData.data(); }

        /// Provides iterator access to the underlying data.
        friend const_iterator begin(const Image& img) { return img.mData.data(); }

        /// Provides iterator access to the underlying data.
        iterator end() { return begin() + mData.size(); }

        /// Provides iterator access to the underlying data.
        const_iterator end() const { return begin() + mData.size(); }

        /// Provides iterator access to the underlying data.
        friend iterator end(Image& img) { return img.end(); }

        /// Provides iterator access to the underlying data.
        friend const_iterator end(const Image& img) { return img.end(); }

        /// Converts the image "src" to a given color format and saves the result to "dest". One
        /// couild pass the same object as both "src" and "dest", but doing so is ineffective,
        /// unless the conversion is YUV420SP to GRAY. Only some conversions are supported, namely:
        /// GRAY to BGR, BGR to GRAY, YUV420SP to BGR, YUV420SP to GRAY.
        static void convert(const Image& src, Image& dest, Format format);

        /// Swaps the contents of the two Image instances.
        void swap(Image& rhs);

        /// Swaps the contents of the two Image instances.
        friend void swap(Image& lhs, Image& rhs);

    private:
        /// Wraps the data pointer in a Mat object, after ensuring that the underlying array is
        /// large enough to hold image data of the required size.
        cv::Mat resize(Format format, Dims dims);

        /// Wraps the data pointer in a Mat object. Be careful with this one -- use the returned Mat
        /// only for reading.
        cv::Mat wrap() const;

        // data
        std::vector<uint8_t> mData;
        Dims mDims = {0, 0};
        Format mFormat = Format::UNKNOWN;
    };
}

#endif // FMO_IMAGE_HPP
