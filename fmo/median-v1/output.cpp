#include "../include-opencv.hpp"
#include "algorithm-median.hpp"
#include <array>

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
        out.offset = -2;

        for (auto& o : mObjects[2]) {
            if (!o.selected) { continue; }

            out.detections.emplace_back();
            if (o.prev != Special::END) {
                auto& oPrev = mObjects[3][o.prev];
                out.detections.back().reset(new MyDetection(this, getBounds(o), &o, &oPrev));
            } else {
                out.detections.back().reset(new MyDetection(this, getBounds(o), &o));
            }
        }
    }

    MedianV1::MyDetection::MyDetection(MedianV1* aMe, Bounds bounds, const Object* obj,
                                       const Object* objPrev)
        : Detection(obj->center, objPrev->center, obj->halfLen[1]),
          me(aMe),
          mBounds(bounds),
          mObj(obj) {}

    MedianV1::MyDetection::MyDetection(MedianV1* aMe, Bounds bounds, const Object* obj)
        : Detection(obj->center, obj->halfLen[1]), me(aMe), mBounds(bounds), mObj(obj) {}

    void MedianV1::MyDetection::getPoints(PointSet& out) const {
        // apply radius correction
        float radius = mObj->halfLen[1];
        // radius = mCfg->outputRadiusLinear * radius + mCfg->outputRadiusConstant;
        // radius = std::max(radius, mCfg->outputRadiusMin);
        {
            float step = float(1 << me->mProcessingLevel.pixelSizeLog2);
            float arg = radius / step;
            constexpr std::array<float, 9> points = {-1e6f, 0.f, 0.5f, 1.f, 2.f,
                                                     4.f,   8.f, 16.f, 1e6f};
            std::array<float, 9> values = {1.f,
                                           1.f,
                                           me->mCfg.outputRadiusCorr[0],
                                           me->mCfg.outputRadiusCorr[1],
                                           me->mCfg.outputRadiusCorr[2],
                                           me->mCfg.outputRadiusCorr[3],
                                           me->mCfg.outputRadiusCorr[4],
                                           1.f,
                                           1.f};

            size_t i = std::lower_bound(begin(points), end(points), arg) - begin(points);
            float blend = (arg - points[i - 1]) / (points[i] - points[i - 1]);
            float value = (1 - blend) * values[i - 1] + blend * values[i];
            radius *= value;
        }

        // rasterize the object into a temporary buffer
        Dims dims{mBounds.max.x - mBounds.min.x + 1, mBounds.max.y - mBounds.min.y + 1};
        auto& temp = me->mCache.pointsRaster;
        temp.resize(Format::GRAY, dims);
        cv::Point2f center{float(mObj->center.x - mBounds.min.x),
                           float(mObj->center.y - mBounds.min.y)};
        cv::Point2f a{mObj->direction.x, mObj->direction.y};
        a *= (mObj->halfLen[0] - mObj->halfLen[1]);
        cv::Point2f p1 = center - a;
        cv::Point2f p2 = center + a;
        cv::Mat buf = temp.wrap();
        buf.setTo(uint8_t(0x00));
        int thickness = std::max(1, int(std::round(2.f * radius)));
        cv::line(buf, p1, p2, 0xFF, thickness);

        // output non-zero points
        out.clear();
        const uint8_t* data = temp.data();
        for (int y = mBounds.min.y; y <= mBounds.max.y; y++) {
            for (int x = mBounds.min.x; x <= mBounds.max.x; x++, data++) {
                if (*data != 0) { out.push_back(Pos{x, y}); }
            }
        }

        // no need to sort the points, they are already sorted according to pointSetCompLt()
    }
}
