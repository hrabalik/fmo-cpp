#include <fmo/assert.hpp>
#include <fmo/image.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace fmo {

    struct Image::Hidden {
        cv::Mat mMat;
    };

    Image::Image() {
        static_assert(sizeof(Image) >= sizeof(Image::Hidden),
                      "Image must be larger than Image::Hidden");
        static_assert(alignof(Image) >= alignof(Image::Hidden),
                      "Image must have equivalent or stricter alignment than Image::Hidden");
        new (&hidden()) Image::Hidden{};
    }

    Image::~Image() { hidden().~Hidden(); }

    Image::Hidden& Image::hidden() { return reinterpret_cast<Hidden&>(mHidden); }

    const Image::Hidden& Image::hidden() const { return reinterpret_cast<const Hidden&>(mHidden); }

    Image::Image(const Image& rhs) : Image() { hidden().mMat = rhs.hidden().mMat.clone(); }

    Image& Image::operator=(const Image& rhs) {
        hidden().mMat = rhs.hidden().mMat.clone();
        return *this;
    }

    Image::Image(Image&& rhs) : Image() { hidden().mMat = rhs.hidden().mMat; }

    Image& Image::operator=(Image&& rhs) {
        hidden().mMat = rhs.hidden().mMat;
        return *this;
    }

    Image::Image(const std::string& filename, Format format) : Image() {
        switch (format) {
        case Format::BGR:
            hidden().mMat = cv::imread(filename, cv::IMREAD_COLOR);
            break;
        case Format::GRAY:
            hidden().mMat = cv::imread(filename, cv::IMREAD_GRAYSCALE);
            break;
        default:
            throw std::runtime_error("reading image: unsupported format");
            break;
        }

        if (hidden().mMat.data == nullptr) { throw std::runtime_error("failed to open image"); }
        mFormat = format;
        int* size = hidden().mMat.size.p;
        mSize = {size[1], size[0]};
    }

    uint8_t* Image::data() { return hidden().mMat.data; }

    const uint8_t* Image::data() const { return hidden().mMat.data; }

    void convert(const Image& src, Image& dest, Image::Format format) {
        enum class Status {
            ERROR,
            GOOD,
            NO_OP,
        } status = Status::ERROR;

        switch (src.mFormat) {
        case Image::Format::BGR:
            switch (format) {
            case Image::Format::BGR:
                status = Status::NO_OP;
                break;
            case Image::Format::GRAY:
                cv::cvtColor(src.hidden().mMat, dest.hidden().mMat, cv::COLOR_BGR2GRAY, 1);
                status = Status::GOOD;
                break;
            default:
                break;
            }
            break;
        case Image::Format::GRAY:
            switch (format) {
            case Image::Format::BGR:
                cv::cvtColor(src.hidden().mMat, dest.hidden().mMat, cv::COLOR_GRAY2BGR, 3);
                status = Status::GOOD;
            case Image::Format::GRAY:
                status = Status::NO_OP;
                break;
            default:
                break;
            }
            break;
        case Image::Format::YUV420SP:
            switch (format) {
            case Image::Format::BGR:
                cv::cvtColor(src.hidden().mMat, dest.hidden().mMat, cv::COLOR_YUV420sp2BGR, 3);
                status = Status::GOOD;
                break;
            case Image::Format::GRAY:
                // todo hidden().mMat = hidden().mMat(...);
                break;
            case Image::Format::YUV420SP:
                status = Status::NO_OP;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }

        if (status == Status::GOOD) {
            dest.mFormat = format;
            dest.mSize = src.mSize;
        } else if (status != Status::NO_OP) {
            throw std::runtime_error("failed to perform color conversion");
        }
    }
}
