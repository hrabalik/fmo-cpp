#ifndef FMO_EXPLORER_IMPL_HPP
#define FMO_EXPLORER_IMPL_HPP

#include <fmo/agglomerator.hpp>
#include <fmo/algorithm.hpp>
#include <fmo/decimator.hpp>
#include <fmo/stats.hpp>
#include <fmo/stripgen.hpp>

namespace fmo {
    /// Implementation details of class Explorer.
    struct ExplorerV2 final : public Algorithm {
        virtual ~ExplorerV2() override;

        /// Initializes all caches. Creates as many decimation levels as needed to process images
        /// with specified dimensions. The following calls to setInputSwap() will require that the
        /// format and dimensions match the format and dimensions provided here.
        ExplorerV2(const Config& cfg, Format format, Dims dims);

        /// To be called every frame, providing the next image for processing. The processing will
        /// take place during the call and might take a long time. The input is received by swapping
        /// the contents of the provided input image with an internal buffer.
        virtual void setInputSwap(Image& input) override;

        /// Visualizes the result of detection, returning an image that is useful for debugging
        /// algorithm behavior. The returned image will have BGR format and the same dimensions as
        /// the input image.
        virtual const Image& getDebugImage() override {
            visualize();
            return mCache.visColor;
        }

        /// Determines whether a new object has been found as a result of analyzing the last frame
        /// during a call to setInputSwap(). When this method returns true, the methods
        /// getObjectBounds() and getObjectDetails() may be called to get more information about the
        /// detected object.
        virtual bool haveObject() const override { return !mObjects.empty(); }

        /// Provides the bounding box that encloses the detected object. Use the haveObject() method
        /// first to check if an object has been detected in this frame.
        virtual Bounds getObjectBounds() const override;

        /// Provides detailed information about the detected object, including a list of object
        /// pixels. Use the haveObject() method first to check if an object has been detected in
        /// this frame.
        virtual void getObjectDetails(ObjectDetails& details) const override;

    private:
        /// Data related to source images.
        struct SourceLevel {
            Format format; ///< source format
            Dims dims;     ///< source dimensions
            Image image1;  ///< newest source image
            Image image2;  ///< source image from previous frame
            Image image3;  ///< source image from two frames before
        };

        /// Data related to decimation levels that will not be processed processed any further.
        /// Serves as a cache during decimation.
        struct IgnoredLevel {
            Image image;
        };

        /// Data related to decimation levels that will be processed. Holds all data required to
        /// detect strips in this frame, as well as some detection results.
        struct ProcessedLevel {
            Image image1;       ///< newest source image
            Image image2;       ///< source image from previous frame
            Image image3;       ///< source image from two frames before
            Image diff1;        ///< newest difference image
            Image diff2;        ///< difference image from previous frame
            Image preprocessed; ///< image ready for strip detection
            int step;           ///< relative pixel width (due to downscaling)
        };

        /// Repurpose the unused width information as the index of the next strip in component.
        static int16_t next(const StripRepr& strip) { return strip.halfDims.width; }

        /// Repurpose the unused width information as the index of the next strip in component.
        static int16_t& next(StripRepr& strip) { return strip.halfDims.width; }

        /// Special values of next().
        enum Special : int16_t {
            UNTOUCHED = 0,
            TOUCHED = 1,
            END = -1,
        };

        /// Connected component data.
        struct Component {
            Component(int16_t aFirst) : first(aFirst) {}

            int16_t first; ///< index of the first strip in component
        };

        /// Cluster data.
        struct Cluster {
            /// A cluster's left or right endpoint.
            struct Endpoint {
                int strip; ///< index of the strip at endpoint
                Pos pos;   ///< position of endpoint
            } l, r;

            int numStrips;         ///< total number of strips in cluster
            float approxHeightMin; ///< minimum approximate strip height of components in cluster
            float approxHeightMax; ///< maximum approximate strip height of components in cluster
            float lengthTotal;     ///< approximate length of cluster
            float lengthGaps;      ///< length of all gaps in cluster
            Bounds bounds1;        ///< bounding box around cluster in T and T-1
            Bounds bounds2;        ///< bounding box around cluster in T-1 and T-2

            enum Reason {
                MERGED = -1,
                TOO_FEW_STRIPS = -2,
                TOO_SHORT = -3,
                NOT_AN_OBJECT = -4,
            };

            void setInvalid(Reason reason) { numStrips = int(reason); }
            Reason whyInvalid() const { return Reason(numStrips); }
            bool isInvalid() const { return numStrips < 0; }
        };

        /// Miscellaneous cached objects, typically accessed by a single method.
        struct Cache {
            Image visDiffGray;
            Image visDiffColor;
            Image visColor;
            std::vector<int> halfHeights;
            std::vector<std::pair<float, Cluster*>> sortClusters;
            Stats noiseStats{3, 0};
        };

        /// Creates low-resolution versions of the source image using decimation.
        void createLevelPyramid(Image& input);

        /// Applies image-wide operations before strips are detected.
        void preprocess();

        /// Applies image-wide operations before strips are detected.
        void preprocess(ProcessedLevel& level);

        /// Detects strips by iterating over the pixels in the image.
        void findStrips();

        /// Detects strips by iterating over the pixels in the image.
        void findStrips(ProcessedLevel& level);

        /// Creates connected components by joining strips together.
        void findComponents();

        /// Creates clusters by joining components together.
        void findClusters();

        /// Locates an object by selecting the best trajectory.
        void findObjects();

        /// Determines whether the given trajectory should be considered a fast-moving object.
        bool isObject(Cluster&) const;

        /// Finds a bounding box enclosing strips which are present in a given difference image. To
        /// convert coordinates to image space, a step value has to be specified which denotes the
        /// ratio of original image pixels to image pixels in the difference image.
        Bounds findClusterBoundsInDiff(const Cluster& cluster, const Mat& diff, int step) const;

        /// Lists pixels that are covered by the detected object and saves them into the point list
        /// in the argument. This method expects that the bounds are already set for the object.
        void getObjectPixels(ObjectDetails& out) const;

        /// Visualizes the results into the visualization image.
        void visualize();

        // data
        SourceLevel mSourceLevel;                 ///< the level with original images
        Decimator mDecimator;                     ///< for reducing input image resolution
        mutable Differentiator mDiff;             ///< for creating difference images
        NewStripGen mStripGen;                    ///< for generating strips
        Agglomerator mAggl;                       ///< for forming clusters from components
        std::vector<IgnoredLevel> mIgnoredLevels; ///< levels that will not be processed
        ProcessedLevel mLevel;                    ///< the level that will be processed
        std::vector<StripRepr> mStrips;           ///< detected strips, ordered by x coordinate
        std::vector<Component> mComponents;       ///< detected components, ordered by x coordinate
        std::vector<Cluster> mClusters;           ///< detected clusters in no particular order
        std::vector<const Cluster*> mObjects;     ///< objects that have been accepted this frame
        int mFrameNum = 0;    ///< frame number, 1 when processing the first frame
        int mNoiseAdjust = 0; ///< adjust differentiation threshold by this value
        Cache mCache;         ///< miscellaneous cached objects
        const Config mCfg;    ///< configuration settings
    };
}

#endif // FMO_EXPLORER_IMPL_HPP
