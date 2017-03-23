#include "../include-opencv.hpp"
#include "explorer.hpp"
#include <algorithm>
#include <fmo/processing.hpp>
#include <fmo/region.hpp>
#include <limits>
#include <type_traits>

namespace fmo {
    // namespace {
    //     constexpr int BOUNDS_MIN = std::numeric_limits<int16_t>::min();
    //     constexpr int BOUNDS_MAX = std::numeric_limits<int16_t>::max();
    // }

    // void ExplorerV2::findObjects() {
    //     // reorder trajectories by the number of strips, largest will be at the front
    //     std::sort(
    //         begin(mTrajectories), end(mTrajectories),
    //         [](const Trajectory& l, const Trajectory& r) { return l.numStrips > r.numStrips;
    //         });
    //
    //     mObjects.clear();
    //     mRejected.clear();
    //     for (auto& traj : mTrajectories) {
    //         // ignore all trajectories with too few strips
    //         if (traj.numStrips < MIN_STRIPS) break;
    //
    //         // test if the trajectory is interesting
    //         if (isObject(traj)) {
    //             mObjects.push_back(&traj);
    //             break; // assume a single interesting trajectory
    //         } else {
    //             mRejected.push_back(&traj);
    //         }
    //     }
    // }

    // bool ExplorerV2::isObject(Trajectory& traj) const {
    //     // find the bounding box enclosing strips present in the difference images
    //     traj.bounds1 = findTrajectoryBoundsInDiff(traj, mLevel.diff1, mLevel.step);
    //     traj.bounds2 = findTrajectoryBoundsInDiff(traj, mLevel.diff2, mLevel.step);
    //
    //     // condition: both diffs must have *some* strips present
    //     if (traj.bounds1.min.x == BOUNDS_MAX || traj.bounds2.min.x == BOUNDS_MAX) return
    //     false;
    //
    //     // force left-to-right direction
    //     int xMin = mStrips[mComponents[traj.first].first].x;
    //     if (traj.bounds1.min.x != xMin) {
    //         // force left-to-right by swapping ranges
    //         std::swap(traj.bounds1, traj.bounds2);
    //     }
    //
    //     // condition: leftmost strip must be in range1
    //     if (traj.bounds1.min.x != xMin) return false;
    //
    //     // condition: rightmost strip must be in range2
    //     int xMax = mStrips[mComponents[traj.last].last].x;
    //     if (traj.bounds2.max.x != xMax) return false;
    //
    //     // condition: range1 must end a significant distance away from rightmost strip
    //     int minMotion = int(mCfg.minMotion * (xMax - xMin));
    //     if (xMax - traj.bounds1.max.x < minMotion) return false;
    //
    //     // condition: range2 must start a significant distance away from lefmost strip
    //     if (traj.bounds2.min.x - xMin < minMotion) return false;
    //
    //     return true;
    // }

    // Bounds ExplorerV2::findTrajectoryBoundsInDiff(const Trajectory& traj, const Mat& diff,
    //                                               int step) const {
    //     int halfStep = step / 2;
    //     const uint8_t* data = diff.data();
    //     int skip = int(diff.skip());
    //     Bounds result{{BOUNDS_MAX, BOUNDS_MAX}, {BOUNDS_MIN, BOUNDS_MIN}};
    //
    //     // iterate over all strips in trajectory
    //     int compIdx = traj.first;
    //     while (compIdx != Component::NO_COMPONENT) {
    //         const Component& comp = mComponents[compIdx];
    //         int stripIdx = comp.first;
    //         while (stripIdx != Strip::END) {
    //             const Strip& strip = mStrips[stripIdx];
    //             int col = (strip.x - halfStep) / step;
    //             int row = (strip.y - halfStep) / step;
    //             uint8_t val = *(data + (row * skip + col));
    //
    //             // if the center of the strip is in the difference image
    //             if (val != 0) {
    //                 // update bounds
    //                 result.min.x = std::min(result.min.x, int(strip.x));
    //                 result.min.y = std::min(result.min.y, int(strip.y));
    //                 result.max.x = std::max(result.max.x, int(strip.x));
    //                 result.max.y = std::max(result.max.y, int(strip.y));
    //             }
    //             stripIdx = strip.special;
    //         }
    //         compIdx = comp.next;
    //     }
    //
    //     return result;
    // }

    // auto ExplorerV2::findBounds(const Trajectory& traj) const -> Bounds {
    //     Bounds result;
    //     result.min = {BOUNDS_MAX, BOUNDS_MAX};
    //     result.max = {BOUNDS_MIN, BOUNDS_MIN};
    //
    //     const Component* comp = &mComponents[traj.first];
    //     const Strip* firstStrip = &mStrips[comp->first];
    //     while (true) {
    //         const Strip* strip = &mStrips[comp->first];
    //         while (true) {
    //             result.min.y = std::min(result.min.y, strip->y - strip->halfHeight);
    //             result.max.y = std::max(result.max.y, strip->y + strip->halfHeight);
    //             if (strip->special == Strip::END) break;
    //             strip = &mStrips[strip->special];
    //         }
    //         if (comp->next == Component::NO_COMPONENT) break;
    //         comp = &mComponents[comp->next];
    //     }
    //     const Strip* lastStrip = &mStrips[comp->last];
    //
    //     int halfWidth = mLevel.step / 2;
    //     result.min.x = firstStrip->x - halfWidth;
    //     result.max.x = lastStrip->x + halfWidth;
    //     return result;
    // }

    Bounds ExplorerV2::getObjectBounds() const {
        // const Trajectory& traj = *mObjects[0];
        // return grow(traj.bounds1, traj.bounds2);
        return Bounds{};
    }

    void ExplorerV2::getObjectDetails(ObjectDetails&) const {
        // // find the bounding box enclosing the object
        // const Trajectory& traj = *mObjects[0];
        // out.bounds1 = traj.bounds1;
        // out.bounds2 = traj.bounds2;
        //
        // // list object pixels
        // getObjectPixels(out);
    }

    // void fmo::ExplorerV2::getObjectPixels(ObjectDetails& out) const {
    //     out.points.clear();
    //     if (mCfg.objectResolution == Config::PROCESSING) {
    //         const Trajectory& traj = *mObjects[0];
    //         int step = mLevel.step;
    //         int halfStep = step / 2;
    //         const uint8_t* data1 = mLevel.diff1.data();
    //         const uint8_t* data2 = mLevel.diff2.data();
    //         int skip1 = int(mLevel.diff1.skip());
    //         int skip2 = int(mLevel.diff2.skip());
    //
    //         // iterate over all strips in trajectory
    //         int compIdx = traj.first;
    //         while (compIdx != Component::NO_COMPONENT) {
    //             const Component& comp = mComponents[compIdx];
    //             int stripIdx = comp.first;
    //             while (stripIdx != Strip::END) {
    //                 const Strip& strip = mStrips[stripIdx];
    //                 int col = (strip.x - halfStep) / step;
    //                 int row = (strip.y - halfStep) / step;
    //                 uint8_t val1 = *(data1 + (row * skip1 + col));
    //                 uint8_t val2 = *(data2 + (row * skip2 + col));
    //
    //                 // if the center of the strip is in both difference images
    //                 if (val1 != 0 && val2 != 0) {
    //                     // put all pixels in the strip as object pixels
    //                     int ye = strip.y + strip.halfHeight;
    //                     int xe = strip.x + halfStep;
    //
    //                     for (int y = strip.y - strip.halfHeight; y < ye; y++) {
    //                         for (int x = strip.x - halfStep; x < xe; x++) {
    //                             out.points.push_back({x, y});
    //                         }
    //                     }
    //                 }
    //                 stripIdx = strip.special;
    //             }
    //             compIdx = comp.next;
    //         }
    //     } else if (mCfg.objectResolution == Config::SOURCE) {
    //         // create regions containing the bounding box in the source images
    //         Bounds bounds = getObjectBounds();
    //         Pos regPos = bounds.min;
    //         Dims regDims = {bounds.max.x - bounds.min.x, bounds.max.y - bounds.min.y};
    //         ExplorerV2* nonConst = const_cast<ExplorerV2*>(this);
    //         auto im1 = nonConst->mSourceLevel.image1.region(regPos, regDims);
    //         auto im2 = nonConst->mSourceLevel.image2.region(regPos, regDims);
    //         auto im3 = nonConst->mSourceLevel.image3.region(regPos, regDims);
    //
    //         // calculate the intersection of differences in the source image
    //         mDiff(mCfg.diff, im1, im2, out.temp1);
    //         mDiff(mCfg.diff, im2, im3, out.temp2);
    //         out.temp3.resize(im1.format(), im1.dims());
    //         cv::bitwise_and(out.temp1.wrap(), out.temp2.wrap(), out.temp3.wrap());
    //
    //         // output pixels in the intersection of differences as object pixels
    //         using value_type = decltype(out.points)::value_type;
    //         static_assert(sizeof(value_type) == sizeof(cv::Point),
    //                       "out.points must be like vector<cv::Point>");
    //         static_assert(std::is_same<decltype(out.points), std::vector<value_type>>::value,
    //                       "out.points must be like vector<cv::Point>");
    //         auto& vec = reinterpret_cast<std::vector<cv::Point>&>(out.points);
    //         cv::findNonZero(out.temp3.wrap(), vec);
    //
    //         // add offset to transform from region to source image coordinates
    //         for (auto& pt : out.points) {
    //             pt.x += bounds.min.x;
    //             pt.y += bounds.min.y;
    //         }
    //     }
    //
    //     // sort to enable fast comparion with other point lists
    //     std::sort(begin(out.points), end(out.points), pointSetComp);
    // }
}
