#include "fastext/FASTex.hpp"
#include <fmo/processing.hpp>
#include <fmo/detector.hpp>

namespace fmo {
    const size_t LEVELS = 3;
    const size_t PROCESSED_LEVELS = 2;
    const size_t SKIPPED_LEVELS = LEVELS - PROCESSED_LEVELS;

    /// Implementation details of class Detector.
    struct Detector::Impl {
        void setInput(const Mat& src) {
            fmo::pyramid(src, cascade, LEVELS);
        }

        const std::vector<Image>& getDebugImages() {
            return debugVis;
        }

    private:
        std::vector<Image> cascade;
        std::vector<Image> debugVis;
        cmp::FASTextGray fastText;
        std::vector<std::vector<cmp::FastKeyPoint>> keypoints;
    };

    Detector::~Detector() = default;
    Detector::Detector() = default;
    Detector::Detector(Detector&&) = default;
    Detector& Detector::operator=(Detector&&) = default;
    void Detector::setInput(const Mat& src) { mImpl->setInput(src); }
    const std::vector<Image>& Detector::getDebugImages() { return mImpl->getDebugImages(); }
}
