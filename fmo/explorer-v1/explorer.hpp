#ifndef FMO_EXPLORER_IMPL_HPP
#define FMO_EXPLORER_IMPL_HPP

#include <fmo/algorithm.hpp>

namespace fmo {
    /// Implementation details of class Explorer.
    struct ExplorerV1 final : public Algorithm {
        static const uint8_t DIFF_THRESH = 19; ///< threshold value for difference image
        static const int MIN_STRIPS = 12;      ///< minimum strips to detect an object
        virtual ~ExplorerV1() override;

        /// Initializes all caches. Creates as many decimation levels as needed to process images
        /// with dimensions specified in the configuration object.
        ExplorerV1(const Config& cfg);

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
            Image image1; ///< newest source image
            Image image2; ///< source image from previous frame
            Image image3; ///< source image from two frames before
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
            int numStrips = 0;  ///< number of strips detected this frame
        };

        /// Strip data.
        struct Strip {
            enum : int16_t {
                UNTOUCHED = 0,
                TOUCHED = 1,
                END = -1,
            };

            Strip(int16_t aX, int16_t aY, int16_t aHalfHeight)
                : x(aX), y(aY), halfHeight(aHalfHeight), special(UNTOUCHED) {}

            // data
            int16_t x, y;       ///< strip coordinates in the source image
            int16_t halfHeight; ///< strip height in the source image, divided by 2
            int16_t special;    ///< special value, status or index of next strip in stroke
        };

        /// Connected component data.
        struct Component {
            enum : int16_t {
                NO_TRAJECTORY = -1,
                NO_COMPONENT = -1,
            };

            Component(int16_t aFirst) : first(aFirst), trajectory(NO_TRAJECTORY) {}

            int16_t first;            ///< index of first strip
            int16_t last;             ///< index of last strip
            int16_t numStrips;        ///< the number of strips in component
            int16_t approxHalfHeight; ///< median of strip half heights
            int16_t next;             ///< index of next component in trajectory
            int16_t trajectory;       ///< index of assigned trajectory
        };

        /// Trajectory data.
        struct Trajectory {
            Trajectory(int16_t aFirst) : first(aFirst), maxWidth(0) {}

            int16_t first;     ///< index of first component
            int16_t last;      ///< index of last component
            int16_t maxWidth;  ///< width of the largest component
            int16_t numStrips; ///< number of strips in trajectory
        };

        /// Miscellaneous cached objects, typically accessed by a single method.
        struct Cache {
            Image visGray;
            Image visColor;
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

        /// Finds properties of previously found components before trajectory search.
        void analyzeComponents();

        /// Creates trajectories by joining components together.
        void findTrajectories();

        /// Finds properties of previously found trajectories before picking the best one.
        void analyzeTrajectories();

        /// Locates an object by selecting the best trajectory.
        void findObjects();

        /// Determines whether the given trajectory should be considered a fast-moving object.
        bool isObject(const Trajectory&) const;

        /// Finds the range of x-coordinates of strips which are present in a given difference
        /// image. To convert coordinates to image space, a step value has to be specified which
        /// denotes the ratio of original image pixels to diff image pixels.
        std::pair<int, int> findTrajectoryRangeInDiff(const Trajectory& traj, const Mat& diff,
                                                      int step) const;

        /// Finds the bounding box that encloses a given trajectory.
        Bounds findBounds(const Trajectory&) const;

        /// Lists pixels that are covered by the detected object and saves them into the point list
        /// in the argument. This method expects that the bounds are already set for the object.
        void getObjectPixels(ObjectDetails& out) const;

        /// Visualizes the results into the visualization image.
        void visualize();

        // data
        SourceLevel mSourceLevel;                 ///< the level with original images
        std::vector<IgnoredLevel> mIgnoredLevels; ///< levels that will not be processed
        ProcessedLevel mLevel;                    ///< the level that will be processed
        std::vector<Strip> mStrips;               ///< detected strips, ordered by x coordinate
        std::vector<Component> mComponents;       ///< detected components, ordered by x coordinate
        std::vector<Trajectory> mTrajectories;    ///< detected trajectories
        std::vector<int> mSortCache;              ///< for storing and sorting integers
        std::vector<const Trajectory*> mRejected; ///< objects that have been rejected this frame
        std::vector<const Trajectory*> mObjects;  ///< objects that have been accepted this frame
        int mFrameNum = 0; ///< frame number, 1 when processing the first frame
        Cache mCache;      ///< miscellaneous cached objects
        const Config mCfg; ///< configuration settings
    };
}

#endif // FMO_EXPLORER_IMPL_HPP
