#include <algorithm>
#include <fmo/assert.hpp>
#include <fmo/image.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace fmo {
    namespace {
        size_t getBytes(Image::Format format, Image::Size size) {
            size_t result = static_cast<size_t>(size.width) * static_cast<size_t>(size.height);

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
                throw std::runtime_error("getBytes: unsupported format");
            }

            return result;
        }

        cv::Size getCvSize(Image::Format format, Image::Size size) {
            cv::Size result{size.width, size.height};

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

        Image::Size getImageSize(Image::Format format, cv::Size size) {
            Image::Size result{size.width, size.height};

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
        Size size = getImageSize(format, mat.size());
        size_t bytes = mat.elemSize() * mat.total();
        FMO_ASSERT(getBytes(format, size) == bytes, "reading image: unexpected size");
        mData.resize(bytes);
        std::copy(mat.data, mat.data + mData.size(), mData.data());
        mFormat = format;
        mSize = size;
    }

    cv::Mat Image::resize(Format format, Size size) {
        size_t bytes = getBytes(format, size);
        mData.resize(bytes);
        return cv::Mat{getCvSize(format, size), getCvType(format), data()};
    }

    cv::Mat Image::wrap() const {
        auto* ptr = const_cast<uint8_t*>(data());
        return cv::Mat{getCvSize(mFormat, mSize), getCvType(mFormat), ptr};
    }

    // void Image::convert(const Image& src, Image& dest, Format format) {
    //     if (&src == &dest) { throw std::runtime_error("convert: src and dest must be distinct"); }
    // 
    //     enum class Status {
    //         ERROR,
    //         GOOD,
    //     } status = Status::ERROR;
    // 
    //     cv::Mat srcMat = src.wrap();
    //     cv::Mat destMat = dest.resize(format, src.mSize);
    // 
    //     switch (src.mFormat) {
    //     case Image::Format::BGR:
    //         switch (format) {
    //         case Image::Format::BGR:
    //             status = Status::NO_OP;
    //             break;
    //         case Image::Format::GRAY:
    //             cv::cvtColor(src.mat(), dest.mat(), cv::COLOR_BGR2GRAY, 1);
    //             status = Status::GOOD;
    //             break;
    //         default:
    //             break;
    //         }
    //         break;
    //     case Image::Format::GRAY:
    //         switch (format) {
    //         case Image::Format::BGR:
    //             cv::cvtColor(src.mat(), dest.mat(), cv::COLOR_GRAY2BGR, 3);
    //             status = Status::GOOD;
    //         case Image::Format::GRAY:
    //             status = Status::NO_OP;
    //             break;
    //         default:
    //             break;
    //         }
    //         break;
    //     case Image::Format::YUV420SP:
    //         switch (format) {
    //         case Image::Format::BGR:
    //             cv::cvtColor(src.mat(), dest.mat(), cv::COLOR_YUV420sp2BGR, 3);
    //             status = Status::GOOD;
    //             break;
    //         case Image::Format::GRAY:
    //             // todo mat() = mat()(...);
    //             break;
    //         case Image::Format::YUV420SP:
    //             status = Status::NO_OP;
    //             break;
    //         default:
    //             break;
    //         }
    //         break;
    //     default:
    //         break;
    //     }
    // 
    //     switch (status) {
    //     case Status::GOOD:
    //         dest.mFormat = format;
    //         dest.mSize = src.mSize;
    //         break;
    //     default:
    //         throw std::runtime_error("failed to perform color conversion");
    //     }
    // }
}
