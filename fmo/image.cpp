#include <fmo/assert.hpp>
#include <fmo/image.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

namespace fmo {

    struct Image::Body {
        cv::Mat mMat;
        Format mFormat = Format::UNKNOWN;
    };

    Image::Image() {
        static_assert(sizeof(Image) >= sizeof(Image::Body),
                      "Image must be larger than Image::Body");
        static_assert(alignof(double) >= alignof(Image::Body),
                      "Image must have equivalent or stricter alignment than Image::Body");
        new (&body()) Image::Body{};
    }

    Image::~Image() { body().~Body(); }

    Image::Body& Image::body() { return reinterpret_cast<Body&>(mBody); }

    const Image::Body& Image::body() const { return reinterpret_cast<const Body&>(mBody); }

    Image::Image(const Image& rhs) : Image() { body().mMat = rhs.body().mMat.clone(); }

    Image& Image::operator=(const Image& rhs) {
        body().mMat = rhs.body().mMat.clone();
        return *this;
    }

    Image::Image(Image&& rhs) : Image() { body().mMat = rhs.body().mMat; }

    Image& Image::operator=(Image&& rhs) {
        body().mMat = rhs.body().mMat;
        return *this;
    }

    Image::Image(const std::string& filename, Format format) : Image() {
        switch (format) {
        case Format::BGR:
            body().mMat = cv::imread(filename, cv::IMREAD_COLOR);
            break;
        case Format::GRAY:
            body().mMat = cv::imread(filename, cv::IMREAD_GRAYSCALE);
            break;
        case Format::YUV420SP:
            // todo read in color and then convert
            break;
        }

        if (body().mMat.data == nullptr) { throw std::runtime_error("failed to open image"); }

        body().mFormat = format;
    }

    uint8_t* Image::data() {
        return body().mMat.data;
    }

    const uint8_t * Image::data() const {
        return body().mMat.data;
    }
}
