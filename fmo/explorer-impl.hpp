#ifndef FMO_EXPLORER_IMPL_HPP
#define FMO_EXPLORER_IMPL_HPP

#include <fmo/explorer.hpp>

namespace fmo {
    /// Implementation details of class Explorer.
    struct Explorer::Impl {
        static const uint8_t DIFF_THRESH = 19; ///< threshold value for difference image
        static const size_t MIN_STRIPS = 12;   ///< minimum strips to detect an object

        /// Initializes all caches. Creates as many decimation levels as needed to process images
        /// with dimensions specified in the configuration object.
        Impl(const Config& cfg);

        /// Called every frame, providing the next image for processing. The processing will take
        /// place during the call and might take some time.
        void setInput(const Mat& src);

        /// Visualizes the result of detection, returning an image that should be displayed to the
        /// user.
        const Image& getDebugImage() {
            visualize();
            return mVisualized;
        }

    private:
        /// Data related to decimation levels that will not be processed processed any further.
        /// Serves as a cache during decimation.
        struct IgnoredLevel {
            Image image;
        };

        /// Data related to decimation levels that will be processed. Holds all data required to
        /// detect strips in this frame, as well as some detection results.
        struct Level {
            Image image1;       ///< newest source image
            Image image2;       ///< source image from previous frame
            Image image3;       ///< source image from two frames before
            Image diff1;        ///< newest difference image
            Image diff2;        ///< difference image from previous frame
            Image preprocessed; ///< image ready for strip detection
            int step;           ///< relative pixel width (due to downscaling)
            int numStrips;      ///< number of strips detected this frame
        };

        /// Strip data.
        struct Strip {
            enum : int16_t {
                UNTOUCHED = 0,
                TOUCHED = 1,
                END = 0,
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

            int16_t first;    ///< index of first component
            int16_t maxWidth; ///< width of the largest component
            float score;      ///< affinity to be an object of interest, higher is better
        };

        /// Data regarding a fast-moving object.
        struct Bounds {
            Pos min; ///< minimum coordinates of the box that encloses the object
            Pos max; ///< maximum coordinates of the box that encloses the object
        };

        /// Creates low-resolution versions of the source image using decimation.
        void createLevelPyramid(const Mat& src);

        /// Applies image-wide operations before strips are detected.
        void preprocess();

        /// Applies image-wide operations before strips are detected.
        void preprocess(Level& level);

        /// Detects strips by iterating over the pixels in the image.
        void findStrips();

        /// Detects strips by iterating over the pixels in the image.
        void findStrips(Level& level);

        /// Creates connected components by joining strips together.
        void findComponents();

        /// Finds properties of previously found components before trajectory search.
        void analyzeComponents();

        /// Creates trajectories by joining components together.
        void findTrajectories();

        /// Finds properties of previously found trajectories before picking the best one.
        void analyzeTrajectories();

        /// Locates an object by selecting the best trajectory.
        void findObject();

        /// Finds the bounding box that encloses a given trajectory.
        Bounds findBounds(const Trajectory&);

        /// Visualizes the results into the visualization image.
        void visualize();

        // data
        std::vector<IgnoredLevel> mIgnoredLevels; ///< levels that will not be processed
        Level mLevel;                             ///< the level that will be processed
        std::vector<Strip> mStrips;               ///< detected strips, ordered by x coordinate
        std::vector<Component> mComponents;       ///< detected components, ordered by x coordinate
        std::vector<Trajectory> mTrajectories;    ///< detected trajectories
        std::vector<int> mSortCache;              ///< for storing and sorting integers
        std::vector<Bounds> mRejectedObjects;     ///< objects that have been rejected this frame
        std::vector<Bounds> mObjects;             ///< objects that have been accepted this frame
        int mFrameNum = 0; ///< frame number, 1 when processing the first frame
        Image mVisualized; ///< visualization image
        const Config mCfg; ///< configuration settings
    };
}

#endif // FMO_EXPLORER_IMPL_HPP
