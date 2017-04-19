#include "../include-opencv.hpp"
#include "algorithm-median.hpp"

namespace fmo {
    bool MedianV1::haveObject() const {
        for (auto& o : mObjects[1]) {
            if (o.selected) return true;
        }
        return false;
    }

    void MedianV1::getObjectDetails(ObjectDetails& out) const {
        auto getBounds = [](const Object& o) {
            NormVector perp = perpendicular(o.direction);
            cv::Point2f a1{o.direction.x, o.direction.y};
            cv::Point2f a2{perp.x, perp.y};
            a1 *= o.halfLen[0];
            a2 *= o.halfLen[1];
            cv::Point2f aMax = {std::max(a1.x, -a1.x) + std::max(a2.x, -a2.x),
                                std::max(a1.y, -a1.y) + std::max(a2.y, -a2.y)};
            cv::Point2f aMin = {std::min(a1.x, -a1.x) + std::min(a2.x, -a2.x),
                                std::min(a1.y, -a1.y) + std::min(a2.y, -a2.y)};
            cv::Point2f cnt{float(o.center.x), float(o.center.y)};
            aMax += cnt;
            aMin += cnt;
            return Bounds{{int(aMin.x), int(aMin.y)}, {int(aMax.x), int(aMax.y)}};
        };

        for (auto& o : mObjects[1]) {
            if (!o.selected) { continue; }
            Bounds b = getBounds(o);

            // rasterize the object into a temporary buffer
            Dims srcDims = mSourceLevel.image.dims();
            Pos last{std::min(b.max.x + 1, srcDims.width), std::min(b.max.y + 1, srcDims.height)};
            Dims dims{last.x - b.min.x, last.y - b.min.y};
            out.temp1.resize(Format::GRAY, dims);
            cv::Point2f center{float(o.center.x - b.min.x), float(o.center.y - b.min.y)};
            cv::Point2f a{o.direction.x, o.direction.y};
            a *= (o.halfLen[0] - o.halfLen[1]);
            cv::Point2f p1 = center - a;
            cv::Point2f p2 = center + a;
            cv::Mat buf = out.temp1.wrap();
            buf.setTo(uint8_t(0x00));
            cv::line(buf, p1, p2, 0xFF, int(2.f * o.halfLen[1]));

            // output points
            out.points.clear();
            const uint8_t* data = out.temp1.data();
            for (int y = b.min.y; y < last.y; y++) {
                for (int x = b.min.x; x < last.x; x++, data++) {
                    if (*data != 0) { out.points.push_back(Pos{x, y}); }
                }
            }

            // store bounds
            out.bounds1 = b;
            out.bounds2 = getBounds(mObjects[2][o.prev]);

            // only one object supported
            break;
        }
    }
}
