#ifndef FMO_STRIPGEN_IMPL_HPP
#define FMO_STRIPGEN_IMPL_HPP

#include <fmo/stripgen.hpp>

namespace fmo {
    template <typename CallbackFunc>
    void StripGen::operator()(const fmo::Mat& img, int minHeight, int minGap, int step,
                              CallbackFunc cb) {
        const Dims dims = img.dims();
        const int skip = int(img.skip());
        const int halfStep = step / 2;

        struct {
            int black2Prev = 0;
            int blackPrev = 0;
            int black = 0;
            int whitePrev = 0;
            int white = 0;
            int col = 0;
            int row = 0;
        } s;

        const uint8_t* colData = img.data();
        noise = 0;

        // Called after a white strip has ended. Stores a strip if the previous two black and
        // one white strip satisfy all conditions.
        auto check = [this, &s, step, halfStep, minGap, cb]() {
            if (s.black2Prev >= minGap && s.blackPrev >= minGap && s.whitePrev > 0) {
                int halfHeight = s.whitePrev * halfStep;
                int x = (s.col * step) + halfStep;
                int y = ((s.row - s.white - s.blackPrev) * step) - halfHeight;
                cb(Pos16{int16_t(x), int16_t(y)}, Dims16{int16_t(halfStep), int16_t(halfHeight)});
            }
        };

        for (s.col = 0; s.col < dims.width; s.col++, colData++) {
            const uint8_t* data = colData;
            s.black2Prev = 0;
            s.blackPrev = 0;
            s.black = minGap; // hack: don't limit by top edge
            s.whitePrev = 0;
            s.white = 0;

            for (s.row = 0; s.row < dims.height; s.row++, data += skip) {
                if (*data != 0) {
                    if (s.white++ == 0) {
                        s.black2Prev = s.blackPrev;
                        s.blackPrev = s.black;
                        s.black = 0;
                    }
                } else {
                    if (s.black++ == 0) {
                        if (s.white >= minHeight) {
                            check();
                            s.whitePrev = s.white;
                            s.white = 0;
                        } else {
                            noise++;
                            s.black = s.blackPrev + s.white + s.black;
                            s.blackPrev = s.black2Prev;
                            s.black2Prev = 0;
                            s.white = 0;
                        }
                    }
                }
            }

            // hack: don't limit by bottom edge
            if (s.white != 0) {
                check();
            } else {
                s.black2Prev = s.blackPrev;
                s.blackPrev = s.black + minGap;
                s.white = 0;
                s.row = s.row + minGap;
                check();
            }
        }
    }
}

#endif // FMO_STRIPGEN_IMPL_HPP
