#include <algorithm>
#include <fmo/assert.hpp>
#include <fmo/image.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace fmo {
    namespace {
        /// Get the number of bytes of data that an image requires, given its format and dimensions.
        size_t getNumBytes(Image::Format format, Image::Dims dims) {
            size_t result = static_cast<size_t>(dims.width) * static_cast<size_t>(dims.height);

            switch (format) {
            case Image::Format::BGR:
                result *= 3;
                break;
            case Image::Format::GRAY:
                break;
            case Image::Format::YUV420SP:
                result = (result * 3) / 2;
                break;
            default:
                throw std::runtime_error("getNumBytes: unsupported format");
            }

            return result;
        }

        /// Convert the actual dimensions to the size that is used by OpenCV. OpenCV considers YUV
        /// 4:2:0 SP images 1.5x taller.
        cv::Size getCvSize(Image::Format format, Image::Dims dims) {
            cv::Size result{dims.width, dims.height};

            switch (format) {
            case Image::Format::BGR:
            case Image::Format::GRAY:
                break;
            case Image::Format::YUV420SP:
                result.height = (result.height * 3) / 2;
                break;
            default:
                throw std::runtime_error("getCvSize: unsupported format");
            }

            return result;
        }

        /// Convert the size used by OpenCV to the actual dimensions. OpenCV considers YUV 4:2:0 SP
        /// images 1.5x taller.
        Image::Dims getDims(Image::Format format, cv::Size size) {
            Image::Dims result{size.width, size.height};

            switch (format) {
            case Image::Format::BGR:
            case Image::Format::GRAY:
                break;
            case Image::Format::YUV420SP:
                result.height = (result.height * 2) / 3;
                break;
            default:
                throw std::runtime_error("getImageSize: unsupported format");
            }

            return result;
        }

        /// Get the Mat data type used by OpenCV that corresponds to the format.
        int getCvType(Image::Format format) {
            switch (format) {
            case Image::Format::BGR:
                return CV_8UC3;
            case Image::Format::GRAY:
            case Image::Format::YUV420SP:
                return CV_8UC1;
            default:
                throw std::runtime_error("getCvType: unsupported format");
            }
        }
    }

    Image::Image(const std::string& filename, Format format) {
        cv::Mat mat;

        switch (format) {
        case Format::BGR:
            mat = cv::imread(filename, cv::IMREAD_COLOR);
            break;
        case Format::GRAY:
            mat = cv::imread(filename, cv::IMREAD_GRAYSCALE);
            break;
        default:
            throw std::runtime_error("reading image: unsupported format");
            break;
        }

        if (mat.data == nullptr) { throw std::runtime_error("failed to open image"); }

        FMO_ASSERT(mat.isContinuous(), "reading image: not continuous")
        FMO_ASSERT(mat.type() == getCvType(format), "reading image: unexpected mat type");
        Dims dims = getDims(format, mat.size());
        size_t bytes = mat.elemSize() * mat.total();
        FMO_ASSERT(getNumBytes(format, dims) == bytes, "reading image: unexpected size");
        mData.resize(bytes);
        std::copy(mat.data, mat.data + mData.size(), mData.data());
        mFormat = format;
        mDims = dims;
    }

    void Image::assign(Format format, Dims dims, const uint8_t * data) {
        size_t bytes = getNumBytes(format, dims);
        mData.resize(bytes);
        std::copy(data, data + bytes, mData.data());
        mDims = dims;
        mFormat = format;
    }

    cv::Mat Image::resize(Format format, Dims dims) {
        size_t bytes = getNumBytes(format, dims);
        mData.resize(bytes);
        return cv::Mat{getCvSize(format, dims), getCvType(format), data()};
    }

    cv::Mat Image::wrap() const {
        auto* ptr = const_cast<uint8_t*>(data());
        return cv::Mat{getCvSize(mFormat, mDims), getCvType(mFormat), ptr};
    }

    void Image::convert(const Image& src, Image& dest, const Format format) {
        if (src.mFormat == format) {
            // no format change -- just copy
            dest = src;
            return;
        }

        if (&src == &dest) {
            if (src.mFormat == Format::YUV420SP && format == Format::GRAY) {
                // same instance and converting YUV420SP to GRAY: easy case
                size_t bytes = getNumBytes(Format::GRAY, dest.mDims);
                dest.mData.resize(bytes);
                dest.mFormat = Format::GRAY;
                return;
            }

            // same instance: convert into a new, temporary Image, then move into dest
            Image temp;
            convert(src, temp, format);
            dest = std::move(temp);
            return;
        }

        enum class Status {
            ERROR,
            GOOD,
        } status = Status::ERROR;

        cv::Mat srcMat = src.wrap();
        cv::Mat destMat = dest.resize(format, src.mDims);

        switch (src.mFormat) {
        case Format::BGR:
            if (format == Format::GRAY) {
                cv::cvtColor(srcMat, destMat, cv::COLOR_BGR2GRAY, 1);
                status = Status::GOOD;
            }
            break;
        case Format::GRAY:
            if (format == Format::BGR) {
                cv::cvtColor(srcMat, destMat, cv::COLOR_GRAY2BGR, 3);
                status = Status::GOOD;
            }
            break;
        case Format::YUV420SP:
            if (format == Format::BGR) {
                cv::cvtColor(srcMat, destMat, cv::COLOR_YUV420sp2BGR, 3);
                status = Status::GOOD;
            } else if (format == Format::GRAY) {
                std::copy(srcMat.data, srcMat.data + dest.mData.size(), destMat.data);
                status = Status::GOOD;
            }
            break;
        default:
            break;
        }

        switch (status) {
        case Status::GOOD:
            FMO_ASSERT(destMat.data == dest.mData.data(), "convert: dest buffer reallocated");
            dest.mFormat = format;
            dest.mDims = src.mDims;
            break;
        default:
            throw std::runtime_error("convert: failed to perform color conversion");
        }
    }
}
