#ifndef FMO_ALGORITHM_MEDIAN_V1_HPP
#define FMO_ALGORITHM_MEDIAN_V1_HPP

#include <fmo/agglomerator.hpp>
#include <fmo/algebra.hpp>
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
        // structures

        /// Special values used instead of indices.
        enum Special : int16_t {
            UNTOUCHED = 0, ///< not processed
            TOUCHED = 1,   ///< processed
            END = -1,      ///< not an index, e.g. a strip is the last in its component
        };

        /// The reason why a component was discarded.
        enum Reason : int16_t {
            TOO_FEW_STRIPS,
            SMALL_AREA,
            LARGE_CONVEX_HULL,
            SMALL_ASPECT,
        };

        /// Connected component data.
        struct Component {
            Component(int16_t aFirst) : first(aFirst) {}

            int16_t first; ///< index of the first strip in component
        };

        /// Object data.
        struct Object {
            Pos endL, endR;       ///< left and right endpoint
            int area;             ///< area of convex hull
            NormVector direction; ///< principal direction
            float size[2];        ///< size in direction: [0] - principal, [1] - perpendicular
            float aspect;         ///< aspect ratio (1 or greater)

            int16_t prev = Special::END; ///< matched component from the previous frame
        };

        // methods

        /// Decimates the input image until it is below a set height; saves the source image and the
        /// decimated image.
        void swapAndDecimateInput(Image& in);

        /// Calculates the per-pixel median of the last three frames to obtain the background.
        /// Creates a binary difference image of background vs. the latest image.
        void computeBinDiff();

        /// Detects strips by iterating over the pixels in the image. Creates connected components
        /// by joining strips together.
        void findComponents();

        // data

        const Config mCfg; ///< configuration received upon construction

        struct {
            Image image;  ///< latest source image
            int frameNum; ///< the number of images received so far
        } mSourceLevel;

        struct {
            int pixelSizeLog2; ///< processing-level pixel size compared to source level, log2
            Image inputs[3];   ///< input images decimated to processing resolution, 0 - newest
            Image background;  ///< median of the last three inputs
            Image binDiff;     ///< binary difference image, latest image vs. background
        } mProcessingLevel;

        struct {
            std::vector<std::unique_ptr<Image>> decimated; ///< cached decimation steps
            Image inputConverted; ///< latest processing input converted to BGR
            Image diffConverted;  ///< latest diff converted to BGR
            Image diffScaled;     ///< latest diff rescaled to source dimensions
            Image visualized;     ///< debug visualization
        } mCache;

        Decimator mDecimator;               ///< decimation tool that handles any image format
        Differentiator mDiff;               ///< for creating the binary difference image
        StripGen mStripGen;                 ///< for finding strips in the difference image
        std::vector<Strip> mStrips;         ///< detected strips, ordered by x coordinate
        std::vector<int16_t> mNextStrip;    ///< indices of the next strip in component
        std::vector<Component> mComponents; ///< connected components
    };
}

#endif // FMO_ALGORITHM_MEDIAN_V1_HPP
