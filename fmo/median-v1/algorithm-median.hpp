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
        virtual const Image& getDebugImage() override;

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

        /// Decimates the input image until it is below a set height; saves the source image and the
        /// decimated image.
        void swapAndDecimateInput(Image& in);

        /// Calculates the per-pixel median of the last three frames to obtain the background.
        void computeBackground();

        // data

        const Config mCfg; ///< configuration received upon construction

        struct {
            Image image;  ///< latest source image
            int frameNum; ///< the number of images received so far
        } mSourceLevel;

        struct {
            Image inputs[3];   ///< input images decimated to processing resolution, 0 - newest
            Image background;  ///< median of the last three inputs
            int pixelSizeLog2; ///< processing-level pixel size compared to source level, log2
        } mProcessingLevel;

        struct {
            std::vector<std::unique_ptr<Image>> decimated; ///< cached decimation steps
            Image converted;  ///< background for debug visualization, BGR
            Image visualized; ///< debug visualization
        } mCache;

        Decimator mDecimator; ///< decimation tool that handles any image format
    };
}

#endif // FMO_ALGORITHM_MEDIAN_V1_HPP
