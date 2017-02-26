#include <algorithm>
#include <fmo/assert.hpp>
#include <fmo/image.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace fmo {
    namespace {
        /// Get the number of bytes of data that an image requires, given its format and dimensions.
        size_t getNumBytes(Format format, Dims dims) {
            size_t result = static_cast<size_t>(dims.width) * static_cast<size_t>(dims.height);

            switch (format) {
            case Format::GRAY:
                break;
            case Format::BGR:
                result *= 3;
                break;
            case Format::INT32:
                result *= 4;
                break;
            case Format::YUV420SP:
                result = (result * 3) / 2;
                break;
            default:
                throw std::runtime_error("getNumBytes: unsupported format");
            }

            return result;
        }

        /// Convert the actual dimensions to the size that is used by OpenCV. OpenCV considers YUV
        /// 4:2:0 SP images 1.5x taller.
        cv::Size getCvSize(Format format, Dims dims) {
            cv::Size result{dims.width, dims.height};
            if (format == Format::YUV420SP) { result.height = (result.height * 3) / 2; }
            return result;
        }

        /// Convert the size used by OpenCV to the actual dimensions. OpenCV considers YUV 4:2:0 SP
        /// images 1.5x taller.
        Dims getDims(Format format, cv::Size size) {
            Dims result{size.width, size.height};
            if (format == Format::YUV420SP) { result.height = (result.height * 2) / 3; }
            return result;
        }

        /// Get the Mat data type used by OpenCV that corresponds to the format.
        int getCvType(Format format) {
            switch (format) {
            case Format::GRAY:
                return CV_8UC1;
            case Format::BGR:
                return CV_8UC3;
            case Format::INT32:
                return CV_32SC1;
            case Format::YUV420SP:
                return CV_8UC1;
            default:
                throw std::runtime_error("getCvType: unsupported format");
            }
        }

        /// Get the interpolation flag used by OpenCV.
        int getCvInterp(Interp interp) {
            switch (interp) {
            case Interp::NEAREST:
                return cv::INTER_NEAREST;
            case Interp::AREA:
                return cv::INTER_AREA;
            default:
                throw std::runtime_error("getCvInterp: unsupported interpolation type");
            }
        }

        /// Get the number of bytes between a color value and the next one. This makes sense only
        /// for interleaved formats, such as BGR.
        size_t getPixelStep(Format format) {
            switch (format) {
            case Format::GRAY:
                return 1;
            case Format::BGR:
                return 3;
            case Format::INT32:
                return 4;
            case Format::YUV420SP:
                throw std::runtime_error("getPixelStep: not applicable to YUV420SP");
            default:
                throw std::runtime_error("getPixelStep: unsupported format");
            }
        }

        /// Access the gray channel of a YUV420SP mat.
        cv::Mat yuv420SPWrapGray(const Mat& mat) {
            Dims dims = mat.dims();
            uint8_t* data = const_cast<uint8_t*>(mat.data());
            return {cv::Size(dims.width, dims.height), CV_8UC1, data};
        }

        /// Access the UV channel of a YUV420SP mat.
        cv::Mat yuv420SPWrapUV(const Mat& mat) {
            Dims dims = mat.dims();
            uint8_t* data = const_cast<uint8_t*>(mat.uvData());
            return {cv::Size(dims.width, dims.height / 2), CV_8UC1, data};
        }
    }

    // Image

    Image::Image(const std::string& filename, Format format) {
        cv::Mat mat;

        switch (format) {
        case Format::BGR:
            mat = cv::imread(filename, cv::IMREAD_COLOR);
            break;
        default:
            mat = cv::imread(filename, cv::IMREAD_GRAYSCALE);
            break;
        }

        if (mat.data == nullptr) { throw std::runtime_error("failed to open image"); }

        FMO_ASSERT(mat.isContinuous(), "reading image: not continuous");
        FMO_ASSERT(mat.type() == getCvType(format), "reading image: unexpected mat type");
        Dims dims = getDims(format, mat.size());
        size_t bytes = mat.elemSize() * mat.total();
        FMO_ASSERT(getNumBytes(format, dims) == bytes, "reading image: unexpected size");
        mData.resize(bytes);
        std::copy(mat.data, mat.data + mData.size(), mData.data());
        mFormat = format;
        mDims = dims;
    }

    void Image::assign(Format format, Dims dims, const uint8_t* data) {
        size_t bytes = getNumBytes(format, dims);
        mData.resize(bytes);
        mDims = dims;
        mFormat = format;
        std::copy(data, data + bytes, mData.data());
    }

    Region Image::region(Pos pos, Dims dims) {
        if (pos.x < 0 || pos.y < 0 || pos.x + dims.width > mDims.width ||
            pos.y + dims.height > mDims.height) {
            throw std::runtime_error("region outside image");
        }

        auto rowStep = static_cast<size_t>(mDims.width);

        if (mFormat == Format::YUV420SP) {
            if (pos.x % 2 != 0 || pos.y % 2 != 0 || dims.width % 2 != 0 || dims.height % 2 != 0) {
                throw std::runtime_error("region: YUV420SP regions must be aligned to 2px");
            }

            uint8_t* start = data();
            start += static_cast<size_t>(pos.x);
            start += rowStep * static_cast<size_t>(pos.y);

            uint8_t* uvStart = uvData();
            uvStart += static_cast<size_t>(pos.x);
            uvStart += rowStep * static_cast<size_t>(pos.y / 2);

            return {Format::YUV420SP, pos, dims, start, uvStart, rowStep};
        } else {
            size_t pixelStep = getPixelStep(mFormat);
            rowStep *= pixelStep;

            uint8_t* start = mData.data();
            start += pixelStep * static_cast<size_t>(pos.x);
            start += rowStep * static_cast<size_t>(pos.y);

            return {mFormat, pos, dims, start, nullptr, rowStep};
        }
    }

    void Image::resize(Format format, Dims dims) {
        size_t bytes = getNumBytes(format, dims);
        mData.resize(bytes);
        mDims = dims;
        mFormat = format;
    }

    cv::Mat Image::wrap() { return {getCvSize(mFormat, mDims), getCvType(mFormat), mData.data()}; }

    cv::Mat Image::wrap() const {
        auto* ptr = const_cast<uint8_t*>(mData.data());
        return {getCvSize(mFormat, mDims), getCvType(mFormat), ptr};
    }

    // Region

    Region::Region(Format format, Pos pos, Dims dims, uint8_t* data, uint8_t* uvData,
                   size_t rowStep)
        : Mat(format, dims), mPos(pos), mData(data), mUvData(uvData), mRowStep(rowStep) {
        if (pos.x < 0 || pos.y < 0 || dims.width < 0 || dims.height < 0) {
            throw std::runtime_error("region: bad constructor arguments");
        }
    }

    Region Region::region(Pos pos, Dims dims) {
        if (pos.x < 0 || pos.y < 0 || pos.x + dims.width > mDims.width ||
            pos.y + dims.height > mDims.height) {
            throw std::runtime_error("sub-region outside region");
        }

        Pos newPos{mPos.x + pos.x, mPos.y + pos.y};

        if (mFormat == Format::YUV420SP) {
            if (pos.x % 2 != 0 || pos.y % 2 != 0 || dims.width % 2 != 0 || dims.height % 2 != 0) {
                throw std::runtime_error("region: YUV420SP regions must be aligned to 2px");
            }

            uint8_t* start = mData;
            start += static_cast<size_t>(pos.x);
            start += mRowStep * static_cast<size_t>(pos.y);

            uint8_t* uvStart = mUvData;
            uvStart += static_cast<size_t>(pos.x);
            uvStart += mRowStep * static_cast<size_t>(pos.y / 2);

            return {mFormat, newPos, dims, start, uvStart, mRowStep};
        } else {
            uint8_t* start = mData;
            start += getPixelStep(mFormat) * static_cast<size_t>(pos.x);
            start += mRowStep * static_cast<size_t>(pos.y);

            return {mFormat, newPos, dims, start, nullptr, mRowStep};
        }
    }

    void Region::resize(Format format, Dims dims) {
        if (mFormat != format || mDims != dims) {
            throw std::runtime_error("resize: regions mustn't change format or size");
        }
    }

    cv::Mat Region::wrap() {
        if (mFormat == Format::YUV420SP) {
            throw std::runtime_error("wrap: cannot wrap YUV420SP regions");
        }
        return {getCvSize(mFormat, mDims), getCvType(mFormat), mData, mRowStep};
    }

    cv::Mat Region::wrap() const {
        if (mFormat == Format::YUV420SP) {
            throw std::runtime_error("wrap: cannot wrap YUV420SP regions");
        }
        return {getCvSize(mFormat, mDims), getCvType(mFormat), mData, mRowStep};
    }

    // functions

    void save(const Mat& src, const std::string& filename) {
        cv::Mat srcMat = src.wrap();
        cv::imwrite(filename, srcMat);
    }

    void copy(const Mat& src, Mat& dst) {
        dst.resize(src.format(), src.dims());

        if (src.format() == Format::YUV420SP) {
            cv::Mat srcMat1 = yuv420SPWrapGray(src);
            cv::Mat srcMat2 = yuv420SPWrapUV(src);
            cv::Mat dstMat1 = yuv420SPWrapGray(dst);
            cv::Mat dstMat2 = yuv420SPWrapUV(dst);
            srcMat1.copyTo(dstMat1);
            srcMat2.copyTo(dstMat2);
        } else {
            cv::Mat srcMat = src.wrap();
            cv::Mat dstMat = dst.wrap();
            srcMat.copyTo(dstMat);
        }
    }

    void convert(const Mat& src, Mat& dst, Format format) {
        const auto srcFormat = src.format();
        const auto dstFormat = format;

        if (srcFormat == dstFormat) {
            // no format change -- just copy
            copy(src, dst);
            return;
        }

        if (&src == &dst) {
            if (srcFormat == dstFormat) {
                // same instance and no format change: no-op
                return;
            }

            if (srcFormat == Format::YUV420SP && dstFormat == Format::GRAY) {
                // same instance and converting YUV420SP to GRAY: easy case
                dst.resize(Format::GRAY, dst.dims());
                return;
            }

            // same instance: convert into a new, temporary Image, then move into dst
            Image temp;
            convert(src, temp, dstFormat);
            dst = std::move(temp);
            return;
        }

        enum { ERROR = -1 };
        int code = ERROR;

        dst.resize(dstFormat, src.dims()); // this is why we check for same instance
        cv::Mat srcMat = src.wrap();
        cv::Mat dstMat = dst.wrap();

        if (srcFormat == Format::BGR) {
            if (dstFormat == Format::GRAY) { code = cv::COLOR_BGR2GRAY; }
        } else if (srcFormat == Format::GRAY) {
            if (dstFormat == Format::BGR) { code = cv::COLOR_GRAY2BGR; }
        } else if (srcFormat == Format::YUV420SP) {
            if (dstFormat == Format::BGR) {
                code = cv::COLOR_YUV420sp2BGR;
            } else if (dstFormat == Format::GRAY) {
                code = cv::COLOR_YUV420sp2GRAY;
            }
        }

        if (code == ERROR) {
            throw std::runtime_error("convert: failed to perform color conversion");
        }

        cv::cvtColor(srcMat, dstMat, code);

        FMO_ASSERT(dstMat.data == dst.data(), "convert: dst buffer reallocated");
    }

    void less_than(const Mat& src, Mat& dst, uint8_t value) {
        if (src.format() != Format::GRAY) {
            throw std::runtime_error("less_than: input must be GRAY");
        }

        dst.resize(Format::GRAY, src.dims());
        cv::Mat srcMat = src.wrap();
        cv::Mat dstMat = dst.wrap();

        cv::threshold(srcMat, dstMat, value - 1, 0xFF, cv::THRESH_BINARY_INV);
        FMO_ASSERT(dstMat.data == dst.data(), "less_than: dst buffer reallocated");
    }

    void equal(const Mat& src, Mat& dst, uint8_t value) {
        if (src.format() != Format::GRAY) { throw std::runtime_error("equal: input must be GRAY"); }

        dst.resize(Format::GRAY, src.dims());
        cv::Mat srcMat = src.wrap();
        cv::Mat dstMat = dst.wrap();

        dstMat = srcMat == value;
        FMO_ASSERT(dstMat.data == dst.data(), "equal: dst buffer reallocated");
    }

    std::pair<uint8_t*, uint8_t*> min_max(Mat& src) {
        if (src.format() != Format::GRAY) {
            throw std::runtime_error("min_max: input must be GRAY");
        }

        cv::Mat srcMat = src.wrap();
        cv::Point min, max;
        cv::minMaxLoc(srcMat, nullptr, nullptr, &min, &max);
        return {&srcMat.at<uint8_t>(min), &srcMat.at<uint8_t>(max)};
    }

    void absdiff(const Mat& src1, const Mat& src2, Mat& dst) {
        if (src1.format() != src2.format()) {
            throw std::runtime_error("diff: inputs must have same format");
        }
        if (src1.dims() != src2.dims()) {
            throw std::runtime_error("diff: inputs must have same size");
        }

        dst.resize(src1.format(), src1.dims());
        cv::Mat src1Mat = src1.wrap();
        cv::Mat src2Mat = src2.wrap();
        cv::Mat dstMat = dst.wrap();

        cv::absdiff(src1Mat, src2Mat, dstMat);
        FMO_ASSERT(dstMat.data == dst.data(), "resize: dst buffer reallocated");
    }

    void deltaYUV420SP(const Mat& src1, const Mat& src2, Mat& dst) {
        if (src1.format() != Format::YUV420SP || src2.format() != Format::YUV420SP) {
            throw std::runtime_error("delta: inputs must be YUV420SP");
        }

        Dims dims = src1.dims();

        if (src2.dims() != dims) { throw std::runtime_error("delta: inputs must have same size"); }

        dst.resize(Format::GRAY, src1.dims());
        cv::Mat src1Mat = yuv420SPWrapGray(src1);
        cv::Mat src2Mat = yuv420SPWrapGray(src2);
        cv::Mat dstMat = dst.wrap();

        cv::absdiff(src1Mat, src2Mat, dstMat);
        cv::threshold(dstMat, dstMat, 19, 0xFF, cv::THRESH_BINARY);
    }

    void decimate(const Mat& src, Mat& dst, Interp interp) {
        if (src.format() == Format::YUV420SP) {
            throw std::runtime_error("dowscale: source cannot be YUV420SP");
        }

        Dims srcDims = src.dims();

        if (srcDims.width % 2 != 0 || srcDims.height % 2 != 0) {
            throw std::runtime_error("downscale: source dimensions must be divisible by 2");
        }

        Dims dstDims = {srcDims.width / 2, srcDims.height / 2};
        dst.resize(src.format(), dstDims);
        cv::Mat srcMat = src.wrap();
        cv::Mat dstMat = dst.wrap();

        cv::resize(srcMat, dstMat, cv::Size(dstDims.width, dstDims.height), 0, 0,
                   getCvInterp(interp));
    }
}
