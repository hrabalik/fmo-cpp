#include "../include-opencv.hpp"
#include "explorer.hpp"
#include <algorithm>
#include <fmo/processing.hpp>
#include <fmo/region.hpp>
#include <limits>
#include <type_traits>

namespace fmo {
    namespace {
        constexpr int int_min = std::numeric_limits<int>::min();
        constexpr int int_max = std::numeric_limits<int>::max();
    }

    void ExplorerV1::findObjects() {
        // reorder trajectories by the number of strips, largest will be at the front
        std::sort(
            begin(mTrajectories), end(mTrajectories),
            [](const Trajectory& l, const Trajectory& r) { return l.numStrips > r.numStrips; });

        mObjects.clear();
        mRejected.clear();
        for (const auto& traj : mTrajectories) {
            // ignore all trajectories with too few strips
            if (traj.numStrips < MIN_STRIPS) break;

            // test if the trajectory is interesting
            if (isObject(traj)) {
                mObjects.push_back(&traj);
                break; // assume a single interesting trajectory
            } else {
                mRejected.push_back(&traj);
            }
        }
    }

    bool ExplorerV1::isObject(const Trajectory& traj) const {
        // find the range of x-coordinates of strips present in the difference images
        auto range1 = findTrajectoryRangeInDiff(traj, mLevel.diff1, mLevel.step);
        auto range2 = findTrajectoryRangeInDiff(traj, mLevel.diff2, mLevel.step);

        // condition: both diffs must have *some* strips present
        if (range1.first == int_max || range2.first == int_max) return false;

        // force left-to-right direction
        int xMin = mStrips[mComponents[traj.first].first].x;
        if (range1.first != xMin) {
            // force left-to-right by swapping ranges
            std::swap(range1, range2);
        }

        // condition: leftmost strip must be in range1
        if (range1.first != xMin) return false;

        // condition: rightmost strip must be in range2
        int xMax = mStrips[mComponents[traj.last].last].x;
        if (range2.second != xMax) return false;

        // condition: range1 must end a significant distance away from rightmost strip
        int minMotion = int(mCfg.minMotion * (xMax - xMin));
        if (xMax - range1.second < minMotion) return false;

        // condition: range2 must start a significant distance away from lefmost strip
        if (range2.first - xMin < minMotion) return false;

        return true;
    }

    std::pair<int, int> ExplorerV1::findTrajectoryRangeInDiff(const Trajectory& traj,
                                                                  const Mat& diff, int step) const {
        int halfStep = step / 2;
        const uint8_t* data = diff.data();
        int skip = int(diff.skip());
        int first = int_max;
        int last = int_min;

        // iterate over all strips in trajectory
        int compIdx = traj.first;
        while (compIdx != Component::NO_COMPONENT) {
            const Component& comp = mComponents[compIdx];
            int stripIdx = comp.first;
            while (stripIdx != Strip::END) {
                const Strip& strip = mStrips[stripIdx];
                int col = (strip.x - halfStep) / step;
                int row = (strip.y - halfStep) / step;
                uint8_t val = *(data + (row * skip + col));

                // if the center of the strip is in the difference image
                if (val != 0) {
                    // update first, last
                    first = std::min(first, int(strip.x));
                    last = std::max(last, int(strip.x));
                }
                stripIdx = strip.special;
            }
            compIdx = comp.next;
        }

        return std::pair<int, int>{first, last};
    }

    auto ExplorerV1::findBounds(const Trajectory& traj) const -> Bounds {
        Bounds result;
        result.min = {int_max, int_max};
        result.max = {int_min, int_min};

        const Component* comp = &mComponents[traj.first];
        const Strip* firstStrip = &mStrips[comp->first];
        while (true) {
            const Strip* strip = &mStrips[comp->first];
            while (true) {
                result.min.y = std::min(result.min.y, strip->y - strip->halfHeight);
                result.max.y = std::max(result.max.y, strip->y + strip->halfHeight);
                if (strip->special == Strip::END) break;
                strip = &mStrips[strip->special];
            }
            if (comp->next == Component::NO_COMPONENT) break;
            comp = &mComponents[comp->next];
        }
        const Strip* lastStrip = &mStrips[comp->last];

        int halfWidth = mLevel.step / 2;
        result.min.x = firstStrip->x - halfWidth;
        result.max.x = lastStrip->x + halfWidth;
        return result;
    }

    Bounds ExplorerV1::getObjectBounds() const { return findBounds(*mObjects[0]); }

    void ExplorerV1::getObjectDetails(ObjectDetails& out) const {
        out.points.clear();

        if (mObjects.empty()) {
            out.bounds.min = {-1, -1};
            out.bounds.max = {-1, -1};
            out.temp1.clear();
            out.temp2.clear();
            out.temp3.clear();
            return;
        }

        // find the bounding box where the object is located
        out.bounds = findBounds(*mObjects[0]);

        // list object pixels
        getObjectPixels(out);
    }

    void fmo::ExplorerV1::getObjectPixels(ObjectDetails& out) const {
        if (mCfg.objectResolution == Config::PROCESSING) {
            const Trajectory& traj = *mObjects[0];
            int step = mLevel.step;
            int halfStep = step / 2;
            const uint8_t* data1 = mLevel.diff1.data();
            const uint8_t* data2 = mLevel.diff2.data();
            int skip1 = int(mLevel.diff1.skip());
            int skip2 = int(mLevel.diff2.skip());

            // iterate over all strips in trajectory
            int compIdx = traj.first;
            while (compIdx != Component::NO_COMPONENT) {
                const Component& comp = mComponents[compIdx];
                int stripIdx = comp.first;
                while (stripIdx != Strip::END) {
                    const Strip& strip = mStrips[stripIdx];
                    int col = (strip.x - halfStep) / step;
                    int row = (strip.y - halfStep) / step;
                    uint8_t val1 = *(data1 + (row * skip1 + col));
                    uint8_t val2 = *(data2 + (row * skip2 + col));

                    // if the center of the strip is in both difference images
                    if (val1 != 0 && val2 != 0) {
                        // put all pixels in the strip as object pixels
                        int ye = strip.y + strip.halfHeight;
                        int xe = strip.x + halfStep;

                        for (int y = strip.y - strip.halfHeight; y < ye; y++) {
                            for (int x = strip.x - halfStep; x < xe; x++) {
                                out.points.push_back({x, y});
                            }
                        }
                    }
                    stripIdx = strip.special;
                }
                compIdx = comp.next;
            }
        } else if (mCfg.objectResolution == Config::SOURCE) {
            // create regions containing the bounding box in the source images
            Pos regPos = out.bounds.min;
            Dims regDims = {out.bounds.max.x - out.bounds.min.x,
                            out.bounds.max.y - out.bounds.min.y};
            ExplorerV1* nonConst = const_cast<ExplorerV1*>(this);
            auto im1 = nonConst->mSourceLevel.image1.region(regPos, regDims);
            auto im2 = nonConst->mSourceLevel.image2.region(regPos, regDims);
            auto im3 = nonConst->mSourceLevel.image3.region(regPos, regDims);

            // calculate the intersection of differences in the source image
            fmo::absdiff(im1, im2, out.temp1);
            fmo::absdiff(im2, im3, out.temp2);
            fmo::greater_than(out.temp1, out.temp1, DIFF_THRESH);
            fmo::greater_than(out.temp2, out.temp2, DIFF_THRESH);
            out.temp3.resize(im1.format(), im1.dims());
            cv::bitwise_and(out.temp1.wrap(), out.temp2.wrap(), out.temp3.wrap());

            // output pixels in the intersection of differences as object pixels
            using value_type = decltype(out.points)::value_type;
            static_assert(sizeof(value_type) == sizeof(cv::Point),
                          "out.points must be like vector<cv::Point>");
            static_assert(std::is_same<decltype(out.points), std::vector<value_type>>::value,
                          "out.points must be like vector<cv::Point>");
            auto& vec = reinterpret_cast<std::vector<cv::Point>&>(out.points);
            cv::findNonZero(out.temp3.wrap(), vec);

            // add offset to transform from region to source image coordinates
            for (auto& pt : out.points) {
                pt.x += out.bounds.min.x;
                pt.y += out.bounds.min.y;
            }
        }

        // sort to enable fast comparion with other point lists
        std::sort(begin(out.points), end(out.points), pointSetComp);
    }
}
