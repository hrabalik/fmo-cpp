#include "explorer.hpp"

namespace fmo {
    void ExplorerV3::findMetaStrips() {
        auto& out = mLevel.metaStrips;
        out.clear();

        // orders strips by their top edge
        auto stripComp = [](const ProtoStrip& l, const ProtoStrip& r) {
            if (l.pos.x == r.pos.x) {
                return (l.pos.y - l.halfDims.height) < (r.pos.y - r.halfDims.height);
            } else {
                return l.pos.x < r.pos.x;
            }
        };

        // decides if strips are in the same column and their y ranges overlap
        auto stripsOverlap = [](const ProtoStrip& l, const ProtoStrip& r) {
            return l.pos.x == r.pos.x && StripBase::overlapY(l, r);
        };

        float maxRatio = mCfg.maxHeightRatioStrips;
        float minRatio = 1.f / maxRatio;
        using It = decltype(mLevel.strips1)::iterator;
        It newer = mLevel.strips1.begin();
        It older = mLevel.strips2.begin();

        while (newer != mLevel.strips1.end() && older != mLevel.strips2.end()) {
            if (!stripsOverlap(*newer, *older)) {
                // no overlap: add the earlier proto-strip as a separate meta-strip
                if (stripComp(*newer, *older)) {
                    out.emplace_back(*newer, true);
                    newer++;
                } else {
                    out.emplace_back(*older, false);
                    older++;
                }
            } else {
                // some overlap: compare heights
                float heightRatio = float(newer->halfDims.height) / float(older->halfDims.height);
                if (heightRatio < minRatio) {
                    newer++;
                } else if (heightRatio > maxRatio) {
                    older++;
                } else {
                    // heights are compatible: merge two proto-strips into one meta-strip
                    out.emplace_back(*newer, *older);
                    newer++;
                    older++;
                }
            }
        }

        while (newer != mLevel.strips1.end()) {
            out.emplace_back(*newer, true);
            newer++;
        }

        while (older != mLevel.strips2.end()) {
            out.emplace_back(*older, false);
            older++;
        }
    }
}
