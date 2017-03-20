#ifndef FMO_DIFFERENTIATOR_HPP
#define FMO_DIFFERENTIATOR_HPP

#include <fmo/common.hpp>
#include <fmo/image.hpp>

namespace fmo {

    /// Computes first-order absolute difference images in various formats.
    struct Differentiator {
        struct Config {
            uint8_t threshGray = 19;
            uint8_t threshBgr = 19;
            uint8_t threshYuv = 19;
        };

        /// Computes first-order absolute difference image in various formats. The inputs must have
        /// the same format and size. The output is resized to match the size of the inputs and its
        /// format is set to GRAY. The output image is binary -- the values are either 0x00 or 0xFF.
        void operator()(const Config& config, const Mat& src1, const Mat& src2, Mat& dst);

    private:
        Image mDiff;
        Image mSum;
    };
}

#endif // FMO_DIFFERENTIATOR_HPP
