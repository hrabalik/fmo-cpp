#include "../include-opencv.hpp"
#include "algorithm-median.hpp"
#include <fmo/processing.hpp>

namespace fmo {
    namespace {
        const cv::Scalar colorDiscarded{0x00, 0x00, 0xC0};
        const cv::Scalar colorGood{0x00, 0xC0, 0x00};
        const cv::Scalar colorObjects[3] = {
            {0x00, 0xC0, 0x00}, {0x00, 0x80, 0x00}, {0x00, 0x40, 0x00},
        };
    }

    const Image& MedianV1::getDebugImage() {
        // convert to BGR
        fmo::copy(mProcessingLevel.binDiff, mCache.diffConverted, Format::BGR);
        fmo::copy(mProcessingLevel.inputs[0], mCache.inputConverted, Format::BGR);

        // scale to source size
        cv::Mat cvDiff;
        cv::Mat cvVis;
        {
            mCache.diffScaled.resize(Format::BGR, mSourceLevel.image.dims());
            mCache.visualized.resize(Format::BGR, mSourceLevel.image.dims());
            cvDiff = mCache.diffScaled.wrap();
            cvVis = mCache.visualized.wrap();
            cv::resize(mCache.diffConverted.wrap(), cvDiff, cvDiff.size(), 0, 0, cv::INTER_NEAREST);
            cv::resize(mCache.inputConverted.wrap(), cvVis, cvVis.size(), 0, 0, cv::INTER_NEAREST);
        }

        // blend diff and input
        cv::addWeighted(cvDiff, 0.5, cvVis, 0.5, 0, cvVis);

        // draw components
        int step = 1 << mProcessingLevel.pixelSizeLog2;
        for (auto& comp : mComponents) {
            const cv::Scalar* color = &colorDiscarded;
            if (comp.status == Component::GOOD) { color = &colorGood; }

            for (int16_t i = comp.first; i != Special::END; i = mNextStrip[i]) {
                Strip& l = mStrips[i];
                {
                    // draw the strip as a rectangle
                    cv::Point p1{l.pos.x - l.halfDims.width, l.pos.y - l.halfDims.height};
                    cv::Point p2{l.pos.x + l.halfDims.width, l.pos.y + l.halfDims.height};
                    cv::rectangle(cvVis, p1, p2, *color);
                }

                int16_t j = mNextStrip[i];
                if (j != Special::END) {
                    Strip& r = mStrips[j];

                    if (!Strip::inContact(l, r, step)) {
                        // draw an interconnection if in the same component but not touching
                        int x1 = l.pos.x + l.halfDims.width;
                        int x2 = r.pos.x - r.halfDims.width;
                        {
                            cv::Point p1{x1, l.pos.y - l.halfDims.height};
                            cv::Point p2{x2, r.pos.y - r.halfDims.height};
                            cv::line(cvVis, p1, p2, *color);
                        }
                        {
                            cv::Point p1{x1, l.pos.y + l.halfDims.height};
                            cv::Point p2{x2, r.pos.y + r.halfDims.height};
                            cv::line(cvVis, p1, p2, *color);
                        }
                    }
                }
            }
        }

        auto toCvPoint2f = [](NormVector nv) { return cv::Point2f{nv.x, nv.y}; };

        // draw objects
        for (int i = 0; i < 3; i++) {
            auto& color = colorObjects[i];
            for (auto& o : mObjects[i]) {
                cv::Point2f cnt{float(o.center.x), float(o.center.y)};
                cv::Point2f a1 = toCvPoint2f(o.direction);
                cv::Point2f a2 = toCvPoint2f(perpendicular(o.direction));
                constexpr float scale = 1.5f;
                a1 *= scale * o.size[0];
                a2 *= scale * o.size[1];
                cv::Point2f p1 = cnt + a1 - a2;
                cv::Point2f p2 = cnt + a1 + a2;
                cv::Point2f p3 = cnt - a1 + a2;
                cv::Point2f p4 = cnt - a1 - a2;
                cv::line(cvVis, p1, p2, color, 2);
                cv::line(cvVis, p2, p3, color, 2);
                cv::line(cvVis, p3, p4, color, 2);
                cv::line(cvVis, p4, p1, color, 2);
            }
        }

        return mCache.visualized;
    }
}
