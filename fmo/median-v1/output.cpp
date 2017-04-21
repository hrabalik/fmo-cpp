#include "../include-opencv.hpp"
#include "algorithm-median.hpp"

namespace fmo {
    Bounds MedianV1::getBounds(const Object& o) const {
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
        Bounds b{{int(aMin.x), int(aMin.y)}, {int(aMax.x), int(aMax.y)}};
        b.min.x = std::max(b.min.x, 0);
        b.min.y = std::max(b.min.y, 0);
        b.max.x = std::min(b.max.x, mSourceLevel.image.dims().width - 1);
        b.max.y = std::min(b.max.y, mSourceLevel.image.dims().height - 1);
        return b;
    }

    void MedianV1::getOutput(Output& out) {
        out.clear();
        for (auto& o : mObjects[1]) {
            if (!o.selected) { continue; }
            auto& oPrev = mObjects[2][o.prev];
            out.emplace_back();
            out.back().reset(new MyDetection(getBounds(o), &o, &oPrev, &mCache.pointsRaster));
        }
    }

    MedianV1::MyDetection::MyDetection(Bounds bounds, const Object* obj, const Object* objPrev,
                                       Image* temp)
        : Detection(obj->center, objPrev->center, obj->halfLen[1]),
          mBounds(bounds),
          mObj(obj),
          mTemp(temp) {}

    void MedianV1::MyDetection::getPoints(PointSet& out) const {
        // rasterize the object into a temporary buffer
        Dims dims{mBounds.max.x - mBounds.min.x + 1, mBounds.max.y - mBounds.min.y};
        mTemp->resize(Format::GRAY, dims);
        cv::Point2f center{float(mObj->center.x - mBounds.min.x),
                           float(mObj->center.y - mBounds.min.y)};
        cv::Point2f a{mObj->direction.x, mObj->direction.y};
        a *= (mObj->halfLen[0] - mObj->halfLen[1]);
        cv::Point2f p1 = center - a;
        cv::Point2f p2 = center + a;
        cv::Mat buf = mTemp->wrap();
        buf.setTo(uint8_t(0x00));
        cv::line(buf, p1, p2, 0xFF, int(2.f * mObj->halfLen[1]));

        // output non-zero points
        out.clear();
        const uint8_t* data = mTemp->data();
        for (int y = mBounds.min.y; y <= mBounds.max.y; y++) {
            for (int x = mBounds.min.x; x <= mBounds.max.x; x++, data++) {
                if (*data != 0) { out.push_back(Pos{x, y}); }
            }
        }
    }

    bool MedianV1::haveObject() const {
        for (auto& o : mObjects[1]) {
            if (o.selected) return true;
        }
        return false;
    }

    void MedianV1::getObjectDetails(ObjectDetails& out) const {
        for (auto& o : mObjects[1]) {
            if (!o.selected) { continue; }
            Bounds b = getBounds(o);

            // rasterize the object into a temporary buffer
            Pos last{b.max.x + 1, b.max.y + 1};
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