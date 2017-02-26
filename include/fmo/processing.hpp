#ifndef FMO_PROCESSING_HPP
#define FMO_PROCESSING_HPP

#include <fmo/image.hpp>

namespace fmo {
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

    /// Resizes an image so that each dimension is divided by two.
    void decimate(const Mat& src, Mat& dst);

    /// Decimates an image repeatedly and saves each of the downscaled versions.
    void pyramid(const Mat& src, std::vector<Image>& dst, size_t levels);
}

#endif // FMO_PROCESSING_HPP
