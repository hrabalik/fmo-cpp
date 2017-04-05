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

        for (int col = 0; col < dims.width; col++, colData++, origX += int16_t(step)) {
            const uint8_t* data = colData;
            mRle.clear();

            // add top of image
            mRle.push_back(-pad);

            // must start with a black segment
            if (*data != 0) { mRle.push_back(0); }
            data += skip;

            // store indices of changes
            for (int row = 1; row < dims.height; row++, data += skip) {
                if (*data != *(data - skip)) {
                    if ((int(mRle.size()) & 1) == 0 && (row - mRle.back()) < minHeight) {
                        // remove noise
                        mRle.pop_back();
                        mNoise++;
                    } else {
                        mRle.push_back(row);
                    }
                }
            }

            // must end with a black segment
            if ((int(mRle.size()) & 1) == 0) { mRle.push_back(dims.height); }

            // add bottom of image
            mRle.push_back(dims.height + pad);

            // report white segments as strips if all conditions are met
            int last = int(mRle.size()) - 2;
            for (int i = 0; i < last; i += 2) {
                if (mRle[i + 1] - mRle[i + 0] >= minGap && mRle[i + 3] - mRle[i + 2] >= minGap) {
                    int halfHeight = (mRle[i + 2] - mRle[i + 1]) * halfStep;
                    int origY = (mRle[i + 2] + mRle[i + 1]) * halfStep;
                    Pos16 center{int16_t(origX), int16_t(origY)};
                    Dims16 halfDims{int16_t(halfStep), int16_t(halfHeight)};
                    cb(center, halfDims);
                }
            }
        }
    }
}

#endif // FMO_STRIPGEN_IMPL_HPP
