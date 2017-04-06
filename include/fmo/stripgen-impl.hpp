#ifndef FMO_STRIPGEN_IMPL_HPP
#define FMO_STRIPGEN_IMPL_HPP

#include <algorithm>
#include <fmo/assert.hpp>
#include <fmo/stripgen.hpp>

namespace fmo {
    template <typename CallbackFunc>
    void StripGen::operator()(const fmo::Mat& img, int minHeight, int minGap, int step,
                              CallbackFunc cb) {
        using batch_t = uint64_t;
        using rle_t = decltype(mRle)::value_type;
        constexpr int WIDTH = int(sizeof(batch_t) / sizeof(uint8_t));
        FMO_ASSERT(int(img.skip()) % WIDTH == 0, "StripGen::operator(): bad skip");

        const Dims dims = img.dims();
        const int arrSz = dims.height + 4;
        const int skip = int(img.skip()) / WIDTH;
        const int halfStep = step / 2;
        const int pad = std::max(0, std::max(minHeight, minGap));
        const batch_t* colData = (const batch_t*)(img.data());
        mNoise = 0;
        int16_t origX = int16_t(halfStep);
        mRle.resize(arrSz * WIDTH);

        rle_t* front[WIDTH];
        for (int w = 0; w < WIDTH; w++) { front[w] = mRle.data() + (w * arrSz); }

        for (int col = 0; col < dims.width; col += WIDTH, colData++) {
            const batch_t* data = colData;
            rle_t* back[WIDTH];
            int n[WIDTH];

            for (int w = 0; w < WIDTH; w++) {
                back[w] = front[w];

                // add top of image
                *back[w] = rle_t(-pad);
                n[w] = 1;
            }

            // must start with a black segment
            if (*data != 0) {
                for (int w = 0; w < WIDTH; w++) {
                    if (((const uint8_t*)(data))[w] != 0) {
                        *++(back[w]) = rle_t(0);
                        n[w]++;
                    }
                }
            }
            data += skip;

            // store indices of changes
            for (int row = 1; row < dims.height; row++, data += skip) {
                const batch_t* prev = data - skip;
                if (*data != *prev) {
                    for (int w = 0; w < WIDTH; w++) {
                        if (((const uint8_t*)(data))[w] != ((const uint8_t*)(prev))[w]) {
                            if ((n[w] & 1) == 0 && (row - *(back[w])) < minHeight) {
                                // remove noise
                                back[w]--;
                                n[w]--;
                                mNoise++;
                            } else {
                                *++(back[w]) = rle_t(row);
                                n[w]++;
                            }
                        }
                    }
                }
            }

            for (int w = 0; w < WIDTH; w++, origX += int16_t(step)) {
                // must end with a black segment
                if ((n[w] & 1) == 0) {
                    *++(back[w]) = rle_t(dims.height);
                    n[w]++;
                }

                // add bottom of image
                *++(back[w]) = rle_t(dims.height + pad);
                n[w]++;

                // report white segments as strips if all conditions are met
                rle_t* lastWhite = back[w] - 1;
                for (rle_t* i = front[w]; i < lastWhite; i += 2) {
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
}

#endif // FMO_STRIPGEN_IMPL_HPP
