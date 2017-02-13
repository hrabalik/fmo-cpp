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
    /// An object that wraps the OpenCV Mat class.
    struct Mat {
        /// Wraps the data pointer in a Mat object.
        virtual cv::Mat wrap() = 0;

        /// Wraps the data pointer in a Mat object. Be careful with this one -- use the returned Mat
        /// only for reading.
        virtual cv::Mat wrap() const = 0;
    };

    /// An image buffer class. Wraps the OpenCV Mat class. Has value semantics, i.e. copying an
    /// instance of Image will perform a copy of the entire image data.
    struct Image : public Mat {
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

        /// Copies the contents from another image.
        Image(const Image& rhs) { assign(rhs.mFormat, rhs.mDims, rhs.mData.data()); }

        /// Copies the contents from another image.
        Image& operator=(const Image& rhs) {
            assign(rhs.mFormat, rhs.mDims, rhs.mData.data());
            return *this;
        }

        /// Swaps the contents of the image with another image.
        Image(Image&& rhs) noexcept { swap(rhs); }

        /// Swaps the contents of the image with another image.
        Image& operator=(Image&& rhs) noexcept {
            swap(rhs);
            return *this;
        }

        /// Reads an image from file and converts it to the desired format.
        Image(const std::string& filename, Format format);

        /// Copies an image from memory.
        Image(Format format, Dims dims, const uint8_t* data) { assign(format, dims, data); }

        /// Copies an image from memory.
        void assign(Format format, Dims dims, const uint8_t* data);

        /// Resizes the image to math the desired format and dimensions. When the size increases,
        /// iterators may get invalidated and all previous contents may be erased.
        void resize(Format format, Dims dims);

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

        /// Converts the image "src" to a given color format and saves the result to "dst". One
        /// could pass the same object as both "src" and "dst", but doing so is ineffective, unless
        /// the conversion is YUV420SP to GRAY. Only some conversions are supported, namely: GRAY to
        /// BGR, BGR to GRAY, YUV420SP to BGR, YUV420SP to GRAY.
        static void convert(const Image& src, Image& dst, Format format);

        /// Swaps the contents of the two Image instances.
        void swap(Image& rhs) noexcept {
            mData.swap(rhs.mData);
            std::swap(mDims, rhs.mDims);
            std::swap(mFormat, rhs.mFormat);
        }

        /// Removes all data and sets the size to zero. Does not deallocate any memory.
        void clear() noexcept {
            mData.clear();
            mDims = {0, 0};
            mFormat = Format::UNKNOWN;
        }

        /// Swaps the contents of the two Image instances.
        friend void swap(Image& lhs, Image& rhs) noexcept { lhs.swap(rhs); }

        /// Wraps the data pointer in a Mat object.
        virtual cv::Mat wrap() override final;

        /// Wraps the data pointer in a Mat object. Be careful with this one -- use the returned Mat
        /// only for reading.
        virtual cv::Mat wrap() const override final;

    private:
        std::vector<uint8_t> mData;
        Dims mDims = {0, 0};
        Format mFormat = Format::UNKNOWN;
    };
}

#endif // FMO_IMAGE_HPP
