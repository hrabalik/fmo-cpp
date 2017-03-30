#ifndef FMO_EXPLORER_IMPL_HPP
#define FMO_EXPLORER_IMPL_HPP

#include <fmo/agglomerator.hpp>
#include <fmo/algorithm.hpp>
#include <fmo/decimator.hpp>
#include <fmo/stats.hpp>

namespace fmo {
    /// Implementation details of class Explorer.
    struct ExplorerV3 final : public Algorithm {
        virtual ~ExplorerV3() override;

        /// Initializes all caches. Creates as many decimation levels as needed to process images
        /// with specified dimensions. The following calls to setInputSwap() will require that the
        /// format and dimensions match the format and dimensions provided here.
        ExplorerV3(const Config& cfg, Format format, Dims dims);

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
        /// Pos using a small data type for coordinates.
        struct MiniPos {
            int16_t x, y;
            MiniPos() = default;
            MiniPos(const Pos& pos) : x(int16_t(pos.x)), y(int16_t(pos.y)) {}
            MiniPos(int16_t aX, int16_t aY) : x(aX), y(aY) {}
            operator Pos() const { return {x, y}; }
        };

        /// Dims using a small data type for sizes.
        struct MiniDims {
            int16_t width, height;
            MiniDims() = default;
            MiniDims(const Dims& dims) : width(int16_t(dims.width)), height(int16_t(dims.height)) {}
            MiniDims(int16_t aWidth, int16_t aHeight) : width(aWidth), height(aHeight) {}
            operator Dims() const { return {width, height}; }
        };

        /// Bounds using a small data type for coordinates.
        struct MiniBounds {
            MiniPos min, max;
            MiniBounds() = default;
            MiniBounds(const Bounds& bounds) : min(bounds.min), max(bounds.max) {}
            MiniBounds(MiniPos aMin, MiniPos aMax) : min(aMin), max(aMax) {}
            operator Bounds() const { return {min, max}; }
        };

        static MiniBounds grow(const MiniBounds& l, const MiniBounds& r) {
            return MiniBounds{MiniPos{std::min(l.min.x, r.min.x), std::min(l.min.y, r.min.y)},
                              MiniPos{std::max(l.max.x, r.max.x), std::max(l.max.y, r.max.y)}};
        }

        /// Strip data.
        struct Strip {
            enum : int16_t {
                UNTOUCHED = 0,
                TOUCHED = 1,
                END = -1,
            };

            Strip(MiniPos aPos, int16_t aHalfHeight)
                : pos(aPos), halfHeight(aHalfHeight), special(UNTOUCHED) {}

            /// Finds out if two strips touch each other, i.e. they belong to the same connected
            /// component.
            static bool inContact(const Strip& l, const Strip& r, int step) {
                int dx = r.pos.x - l.pos.x;
                if (dx > step) return false;
                int dy = (r.pos.y > l.pos.y) ? (r.pos.y - l.pos.y) : (l.pos.y - r.pos.y);
                return dy < l.halfHeight + r.halfHeight;
            }

            // data
            MiniPos pos;        ///< strip coordinates in the source image
            int16_t halfHeight; ///< strip height in the source image, divided by 2
            int16_t special;    ///< special value, status or index of next strip in stroke
        };

        /// Strip is a non-empty image region with a width of 1 pixel in the processing resolution.
        /// In the original resolution, strips are wider.
        struct StripBase {
            StripBase(const StripBase&) = default;
            StripBase(MiniPos aPos, MiniDims aHalfDims) : pos(aPos), halfDims(aHalfDims) {}

            /// Finds out if two strips would overlap if they were in the same column.
            static bool overlapY(const StripBase& l, const StripBase& r) {
                int dy = (r.pos.y > l.pos.y) ? (r.pos.y - l.pos.y) : (l.pos.y - r.pos.y);
                return dy < l.halfDims.height + r.halfDims.height;
            }

            // data
            MiniPos pos;       ///< coordinates of the center of the strip in the source image
            MiniDims halfDims; ///< dimensions of the strip in the source image, divided by 2
        };

        /// Strip generated by searching in a difference image.
        struct ProtoStrip : public StripBase {
            using StripBase::StripBase;
        };

        /// Strip generated by merging ProtoStrips from two consecutive frames.
        struct MetaStrip : public StripBase {
            enum : int16_t {
                UNTOUCHED = 0, ///< strip not processed and not part of a connected component
                TOUCHED = 1,   ///< strip not processed but a part of a connected component
                END = -1,      ///< strip is the last in a component
            };

            /// Create a strip that is present only in the newer or the older difference image.
            MetaStrip(const ProtoStrip& strip, bool aNewer)
                : StripBase(strip), older(!aNewer), newer(aNewer), motion(0) {}

            /// Create a strip that is present both in the newer and the older difference image.
            MetaStrip(const ProtoStrip& aNewer, const ProtoStrip& aOlder)
                : StripBase(MiniPos(aNewer.pos.x, (aNewer.pos.y + aOlder.pos.y) / 2),
                            MiniDims(aNewer.halfDims.width,
                                     (aNewer.halfDims.height + aOlder.halfDims.height) / 2)),
                  older(true),
                  newer(true),
                  motion(aNewer.pos.y - aOlder.pos.y) {}

            // data
            bool older;               ///< strip is present in the older difference image
            bool newer;               ///< strip is present in the newer difference image
            int16_t motion;           ///< change of pos.y between images
            int16_t next = UNTOUCHED; ///< next strip in the connected component/special value
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
                NOT_AN_OBJECT = -3,
            };

            void setInvalid(Reason reason) { numStrips = int(reason); }
            Reason whyInvalid() const { return Reason(numStrips); }
            bool isInvalid() const { return numStrips < 0; }
        };

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
            Image image1;                    ///< newest source image
            Image image2;                    ///< source image from previous frame
            Image image3;                    ///< source image from two frames before
            Image diff1;                     ///< newest difference image
            Image diff2;                     ///< difference image from previous frame
            std::vector<ProtoStrip> strips1; ///< strips in the newest difference image
            std::vector<ProtoStrip> strips2; ///< strips in the difference image from previous frame
            std::vector<MetaStrip> metaStrips; ///< strips created by merging proto-strips
            Image preprocessed;                ///< image ready for strip detection
            int step;                          ///< relative pixel width (due to downscaling)
            int numStrips = 0;                 ///< number of strips detected this frame
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

        /// Detects strips by iterating over all pixels of the latest difference image.
        void findProtoStrips();

        /// Merges proto-strips from the last two frames.
        void findMetaStrips();

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
        Agglomerator mAggl;                       ///< for forming clusters from components
        std::vector<IgnoredLevel> mIgnoredLevels; ///< levels that will not be processed
        ProcessedLevel mLevel;                    ///< the level that will be processed
        std::vector<Strip> mStrips;               ///< detected strips, ordered by x coordinate
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
