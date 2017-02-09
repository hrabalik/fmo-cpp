#ifndef FMO_IMAGE_HPP
#define FMO_IMAGE_HPP

#include <cstdint>
#include <memory>
#include <string>

namespace fmo {
    /// An image buffer class. Wraps the OpenCV Mat class. Has value semantics, i.e. copying an
    /// instance of Image will perform a copy of the entire image data.
    struct alignas(double) Image {
        /// Possible internal color formats.
        enum class Format : char {
            UNKNOWN = 0,
            GRAY,
            BGR,
            YUV420SP,
        };

        ~Image();

        /// Reads an image from file and converts it to the desired format.
        Image(const std::string& filename, Format format);

        /// Completely copies the argument to create a new Image.
        Image(const Image&);

        /// Completely copies the image on the right hand side to the left hand side.
        Image& operator=(const Image&);

        /// Moves image data from the argument without copying.
        Image(Image&&);

        /// Moves image data to the left hand side without copying.
        Image& operator=(Image&&);

    private:
        Image();

        struct Body;
        Body& body();
        const Body& body() const;

        // data
        char mBody[128];
    };
}

#endif // FMO_IMAGE_HPP
