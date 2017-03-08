#include "evaluator.hpp"

template <typename Func1, typename Func2, typename Func3>
void pointSetCompare(const fmo::PointSet& s1, const fmo::PointSet& s2, Func1 s1Extra, Func2 s2Extra,
                     Func3 both) {
    FMO_ASSERT(std::is_sorted(begin(s1), end(s1), fmo::pointSetComp), "s1 not sorted");
    FMO_ASSERT(std::is_sorted(begin(s2), end(s2), fmo::pointSetComp), "s2 not sorted");

    auto i1 = begin(s1);
    auto i1e = end(s1);
    auto i2 = begin(s2);
    auto i2e = end(s2);

    while (i1 != i1e && i2 != i2e) {
        if (fmo::pointSetComp(*i1, *i2)) {
            s1Extra(*i1);
            i1++;
        } else if (fmo::pointSetComp(*i2, *i1)) {
            s2Extra(*i2);
            i2++;
        } else {
            both(*i1);
            i1++;
            i2++;
        }
    }

    while (i1 != i1e) {
        s1Extra(*i1);
        i1++;
    }

    while (i2 != i2e) {
        s2Extra(*i2);
        i2++;
    }
}

void Evaluator::eval(const fmo::PointSet& ps, const fmo::PointSet& gt, cv::Mat vis) {
    int intersection = 0;
    int union_ = 0;

    pointSetCompare(ps, gt,
                    [&](fmo::Pos pt) {
                        vis.at<cv::Vec3b>({pt.x, pt.y}) = {0xFF, 0x00, 0x00};
                        union_++;
                    },
                    [&](fmo::Pos pt) {
                        vis.at<cv::Vec3b>({pt.x, pt.y}) = {0x00, 0x00, 0xFF};
                        union_++;
                    },
                    [&](fmo::Pos pt) {
                        vis.at<cv::Vec3b>({pt.x, pt.y}) = {0x00, 0xFF, 0x00};
                        intersection++;
                        union_++;
                    });

    auto call = [this](Result r) {
        mCount[int(r)]++;
        mResults.push_back(r);
    };

    if (ps.empty()) {
        if (gt.empty()) {
            call(Result::TN);
        } else {
            call(Result::FN);
        }
    } else {
        double iou = double(intersection) / double(union_);
        if (iou > IOU_THRESHOLD) {
            call(Result::TP);
        } else {
            call(Result::FP);
        }
    }
}
