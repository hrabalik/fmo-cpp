#include "../include-opencv.hpp"
#include "algorithm-median.hpp"
#include <fmo/processing.hpp>

namespace fmo {
    namespace {
        const cv::Scalar stripsColor{0xC0, 0x60, 0x00};
        const cv::Scalar componentConnectionColor{0x00, 0xC0, 0xC0};
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
            for (int16_t i = comp.first; i != Special::END; i = mNextStrip[i]) {
                Strip& l = mStrips[i];
                {
                    // draw the strip as a rectangle
                    cv::Point p1{l.pos.x - l.halfDims.width, l.pos.y - l.halfDims.height};
                    cv::Point p2{l.pos.x + l.halfDims.width, l.pos.y + l.halfDims.height};
                    cv::rectangle(cvVis, p1, p2, stripsColor);
                }

                int16_t j = mNextStrip[i];
                if (j != Special::END) {
                    Strip& r = mStrips[j];

                    if (!Strip::inContact(l, r, step)) {
                        // draw an interconnection if in the same component but not touching
                        cv::Point p1{l.pos.x + l.halfDims.width, l.pos.y};
                        cv::Point p2{r.pos.x - r.halfDims.width, r.pos.y};
                        cv::line(cvVis, p1, p2, stripsColor);
                    }
                }
            }
        }

        return mCache.visualized;
    }
}
