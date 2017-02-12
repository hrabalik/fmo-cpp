#include <fmo/assert.hpp>
#include <fmo/image.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace fmo {

    Image::Image() {
        static_assert(sizeof(mHidden) >= sizeof(cv::Mat),
                      "Image::mHidden must be larger than cv::Mat");
        static_assert(alignof(Image) >= alignof(cv::Mat),
                      "Image must have equivalent or stricter alignment than cv::Mat");
        new (&mat()) cv::Mat{};
    }

    Image::~Image() { mat().~Mat(); }

    cv::Mat& Image::mat() { return reinterpret_cast<cv::Mat&>(mHidden); }

    const cv::Mat& Image::mat() const { return reinterpret_cast<const cv::Mat&>(mHidden); }

    Image::Image(const Image& rhs) : Image() { mat() = rhs.mat().clone(); }

    Image& Image::operator=(const Image& rhs) {
        mat() = rhs.mat().clone();
        return *this;
    }

    Image::Image(Image&& rhs) : Image() { mat() = rhs.mat(); }

    Image& Image::operator=(Image&& rhs) {
        mat() = rhs.mat();
        return *this;
    }

    Image::Image(const std::string& filename, Format format) : Image() {
        switch (format) {
        case Format::BGR:
            mat() = cv::imread(filename, cv::IMREAD_COLOR);
            break;
        case Format::GRAY:
            mat() = cv::imread(filename, cv::IMREAD_GRAYSCALE);
            break;
        default:
            throw std::runtime_error("reading image: unsupported format");
            break;
        }

        if (mat().data == nullptr) { throw std::runtime_error("failed to open image"); }
        mFormat = format;
        int* size = mat().size.p;
        mSize = {size[1], size[0]};
    }

    uint8_t* Image::data() { return mat().data; }

    const uint8_t* Image::data() const { return mat().data; }

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
                cv::cvtColor(src.mat(), dest.mat(), cv::COLOR_BGR2GRAY, 1);
                status = Status::GOOD;
                break;
            default:
                break;
            }
            break;
        case Image::Format::GRAY:
            switch (format) {
            case Image::Format::BGR:
                cv::cvtColor(src.mat(), dest.mat(), cv::COLOR_GRAY2BGR, 3);
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
                cv::cvtColor(src.mat(), dest.mat(), cv::COLOR_YUV420sp2BGR, 3);
                status = Status::GOOD;
                break;
            case Image::Format::GRAY:
                // todo mat() = mat()(...);
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
