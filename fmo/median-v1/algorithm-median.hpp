#ifndef FMO_ALGORITHM_MEDIAN_V1_HPP
#define FMO_ALGORITHM_MEDIAN_V1_HPP

#include <fmo/agglomerator.hpp>
#include <fmo/algorithm.hpp>
#include <fmo/decimator.hpp>
#include <fmo/stats.hpp>
#include <fmo/strip.hpp>

namespace fmo {
    struct MedianV1 final : public Algorithm {
        virtual ~MedianV1() override = default;

        /// Initializes all caches. Creates as many decimation levels as needed to process images
        /// with specified dimensions. The following calls to setInputSwap() will require that the
        /// format and dimensions match the format and dimensions provided here.
        MedianV1(const Config& cfg, Format format, Dims dims);

        /// To be called every frame, providing the next image for processing. The processing will
        /// take place during the call and might take a long time. The input is received by swapping
        /// the contents of the provided input image with an internal buffer.
        virtual void setInputSwap(Image&) override;

        /// Visualizes the result of detection, returning an image that is useful for debugging
        /// algorithm behavior. The returned image will have BGR format and the same dimensions as
        /// the input image.
        virtual const Image& getDebugImage() override {
            mCache.visualized.resize(Format::BGR, mSourceLevel.image.dims());
            return mCache.visualized;
        }

        /// Determines whether a new object has been found as a result of analyzing the last frame
        /// during a call to setInputSwap(). When this method returns true, the methods
        /// getObjectBounds() and getObjectDetails() may be called to get more information about the
        /// detected object.
        virtual bool haveObject() const override { return false; }

        /// Provides the bounding box that encloses the detected object. Use the haveObject() method
        /// first to check if an object has been detected in this frame.
        virtual Bounds getObjectBounds() const override { return Bounds{}; }

        /// Provides detailed information about the detected object, including a list of object
        /// pixels. Use the haveObject() method first to check if an object has been detected in
        /// this frame.
        virtual void getObjectDetails(ObjectDetails&) const override {}

    private:
        // methods

        void swapAndDecimateInput(Image& in);

        // data

        const Config mCfg;

        struct {
            Image image; ///< latest source image
        } mSourceLevel;

        struct {
            Image inputs[3];   ///< input images decimated to processing resolution
            int pixelSizeLog2; ///< processing-level pixel size compared to source level, log2
        } mProcessingLevel;

        struct {
            std::vector<Image> decimated;
            Image visualized;
        } mCache;

        Decimator mDecimator;
    };
}

#endif // FMO_ALGORITHM_MEDIAN_V1_HPP
