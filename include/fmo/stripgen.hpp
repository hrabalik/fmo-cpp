#ifndef FMO_STRIPGEN_HPP
#define FMO_STRIPGEN_HPP

#include <fmo/common.hpp>

namespace fmo {
    /// Detects vertical strips by iterating over all pixels in a binary image. Strip is a non-empty
    /// image region with a width of 1 pixel in the processing resolution. In the original
    /// resolution, strips are wider.
    struct StripGen {
        /// Detects vertical strips in the binary image img. Strips shorter than minHeight will be
        /// discarded as noise. The vertical gap between two strips (that are not considered noise)
        /// must be at least minGap, otherwise both strips are discarded. The parameter step is the
        /// ratio of processing-resolution pixels to original-resolution pixels (must be divisible
        /// by 2 for correct results).
        ///
        /// The callback cb must have the following or compatible signature: void(Pos16 pos, Dims16
        /// halfDims). The first parameter is the strip center in the original image coordinates.
        /// The second parameter is the width and height of the strip in original image pixels,
        /// divided by 2.
        template <typename CallbackFunc>
        void operator()(const fmo::Mat& img, int minHeight, int minGap, int step, CallbackFunc cb);

        /// Returns the number of strips discarded due to minHeight in the last frame.
        int getNoise() const { return noise; }

    private:
        int noise; ///< the number of strips discarded due to minHeight
    };
}

#endif // FMO_STRIPGEN_HPP
