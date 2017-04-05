#ifndef FMO_STRIPGEN_IMPL_HPP
#define FMO_STRIPGEN_IMPL_HPP

#include <algorithm>
#include <fmo/stripgen.hpp>

namespace fmo {
    template <typename CallbackFunc>
    void StripGen::operator()(const fmo::Mat& img, int minHeight, int minGap, int step,
                              CallbackFunc cb) {
        const Dims dims = img.dims();
        const int skip = int(img.skip());
        const int halfStep = step / 2;
        const int pad = std::max(0, std::max(minHeight, minGap));
        const uint8_t* colData = img.data();
        mNoise = 0;
        int16_t origX = int16_t(halfStep);
        mRle.resize(dims.height + 4);

        for (int col = 0; col < dims.width; col++, colData++, origX += int16_t(step)) {
            const uint8_t* data = colData;
            int* front = mRle.data();
            int* back = front;
            int n = 0;

            // add top of image
            *back = -pad;
            n++;

            // must start with a black segment
            if (*data != 0) {
                *++back = 0;
                n++;
            }
            data += skip;

            // store indices of changes
            for (int row = 1; row < dims.height; row++, data += skip) {
                if (*data != *(data - skip)) {
                    if ((n & 1) == 0 && (row - *back) < minHeight) {
                        // remove noise
                        back--;
                        n--;
                        mNoise++;
                    } else {
                        *++back = row;
                        n++;
                    }
                }
            }

            // must end with a black segment
            if ((n & 1) == 0) {
                *++back = dims.height;
                n++;
            }

            // add bottom of image
            *++back = dims.height + pad;
            n++;

            // report white segments as strips if all conditions are met
            int* lastWhite = back - 1;
            for (int* i = front; i < lastWhite; i += 2) {
                if (*(i + 1) - *(i + 0) >= minGap && *(i + 3) - *(i + 2) >= minGap) {
                    int halfHeight = (*(i + 2) - *(i + 1)) * halfStep;
                    int origY = (*(i + 2) + *(i + 1)) * halfStep;
                    Pos16 center{int16_t(origX), int16_t(origY)};
                    Dims16 halfDims{int16_t(halfStep), int16_t(halfHeight)};
                    cb(center, halfDims);
                }
            }
        }
    }
}

#endif // FMO_STRIPGEN_IMPL_HPP
