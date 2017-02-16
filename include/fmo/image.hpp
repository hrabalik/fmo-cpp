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
    // forward declarations
    struct Image;
    struct Region;

    /// Possible image color formats.
    enum class Format {
        UNKNOWN = 0,
        GRAY,
        BGR,
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
        Mat(Format format, Dims dims) : mFormat(format), mDims(dims) {}

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

    /// Stores an image in contiguous memory. Has value semantics, i.e. copying aninstance of Image
    /// will perform a copy of the entire image data.
    struct Image final : public Mat {
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

        /// The number of bytes in the image.
        size_t size() const { return mData.size(); }

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

        /// Creates a region that refers to a rectangular area in the image.
        virtual Region region(Pos pos, Dims dims) override;

        /// The number of bytes to advance if one needs to access the next row.
        virtual size_t skip() const override { return mDims.width; }

        /// Provides access to image data.
        virtual uint8_t* data() override { return mData.data(); }

        /// Provides access to image data.
        virtual const uint8_t* data() const override { return mData.data(); }

        /// Provides access to the part of image data where UVis stored. Only for YUV420SP images.
        virtual uint8_t* uvData() override { return data() + (mDims.width * mDims.height); }

        /// Provides access to the part of image data where UVis stored. Only for YUV420SP images.
        virtual const uint8_t* uvData() const override {
            return data() + (mDims.width * mDims.height);
        }

        /// Resizes the image to match the desired format and dimensions. When the size increases,
        /// iterators may get invalidated and all previous contents may be erased.
        virtual void resize(Format format, Dims dims) override;

        /// Wraps the data pointer in a Mat object.
        virtual cv::Mat wrap() override;

        /// Wraps the data pointer in a Mat object. Be careful with this one -- use the returned Mat
        /// only for reading.
        virtual cv::Mat wrap() const override;

    private:
        std::vector<uint8_t> mData;
    };

    /// Refers to a rectangular part of an image. Does not own any data.
    struct Region final : public Mat {
        Region(const Region&) = default;
        Region& operator=(const Region&) = default;
        Region(Format format, Pos pos, Dims dims, uint8_t* data, uint8_t* uvData, size_t rowStep);

        /// Provides information about the position of the region in the original image.
        Pos pos() const { return mPos; }

        /// Creates a new region that refers to a rectangular area inside this region.
        virtual Region region(Pos pos, Dims dims) override;

        /// The number of bytes to advance if one needs to access the next row.
        virtual size_t skip() const override { return mRowStep; }

        /// Provides access to image data.
        virtual uint8_t* data() override { return mData; }

        /// Provides access to image data.
        virtual const uint8_t* data() const override { return mData; }

        /// Provides access to the part of image data where UVis stored. Only for YUV420SP images.
        virtual uint8_t* uvData() override { return mUvData; }

        /// Provides access to the part of image data where UVis stored. Only for YUV420SP images.
        virtual const uint8_t* uvData() const override { return mUvData; }

        /// Resizes the region to match the desired format and dimensions. A region cannot mustn't
        /// be resized -- an exception is thrown if the current format and dimensions don't match
        /// the arguments.
        virtual void resize(Format format, Dims dims) override;

        /// Wraps the data pointer in a Mat object.
        virtual cv::Mat wrap() override;

        /// Wraps the data pointer in a Mat object. Be careful with this one -- use the returned Mat
        /// only for reading.
        virtual cv::Mat wrap() const override;

    private:
        const Pos mPos;
        uint8_t* const mData;
        uint8_t* const mUvData;
        const size_t mRowStep;
    };

    /// Saves an image to file.
    void save(const Mat& src, const std::string& filename);

    /// Copies image data. To accomodate the data from "src", resize() is called on "dst".
    void copy(const Mat& src, Mat& dst);

    /// Converts the image "src" to a given color format and saves the result to "dst". One could
    /// pass the same object as both "src" and "dst", but doing so is ineffective, unless the
    /// conversion is YUV420SP to GRAY. Only some conversions are supported, namely: GRAY to BGR,
    /// BGR to GRAY, YUV420SP to BGR, YUV420SP to GRAY.
    void convert(const Mat& src, Mat& dst, Format format);

    /// Selects pixels that have a value less than the specified value; these are set to 0xFF while
    /// others are set to 0x00. Input image must be GRAY.
    void less_than(const Mat& src1, Mat& dst, uint8_t value);

    /// Selects pixels that have a certain value; these are set to 0xFF while others are set to
    /// 0x00. Input image must be GRAY.
    void equal(const Mat& src, Mat& dst, uint8_t value);

    /// Locates the minimum and maximum value in an image. Input image must be gray.
    std::pair<uint8_t*, uint8_t*> min_max(Mat& src);

    /// Calculates the absolute difference between the two images. Input images must have the same
    /// format and size.
    void absdiff(const Mat& src1, const Mat& src2, Mat& dst);

    /// Calculates the binary difference image. Pixels that are sufficiently different are white in
    /// the output image, similar pixels are black. The inputs must be YUV420SP.
    void deltaYUV420SP(const Mat& src1, const Mat& src2, Mat& dst);
}

#endif // FMO_IMAGE_HPP
