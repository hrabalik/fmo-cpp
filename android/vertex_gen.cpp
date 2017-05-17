#include "java_interface.hpp"
#include "java_classes.hpp"

namespace {
    RenderBuffers::Pos shiftPoint(fmo::Pos center, float dist, fmo::NormVector dir) {
        RenderBuffers::Pos result;
        result.x = float(center.x) + (dist * dir.x);
        result.y = float(center.y) + (dist * dir.y);
        return result;
    }
}

JNIEXPORT void JNICALL Java_cz_fmo_Lib_generateCurve
        (JNIEnv* env, jclass, jobject detObj, jfloatArray rgba, jobject bObj) {
    static constexpr float DECAY_BASE = 0.33f;
    static constexpr float DECAY_RATE = 0.50f;

    // get data
    RenderBuffers b(env, bObj, false);
    RenderBuffers::Color color;
    env->GetFloatArrayRegion(rgba, 0, 4, (float*) &color);
    float decay = 1.f - DECAY_BASE;

    // first vertex
    Detection d(env, detObj, false);
    Detection dNext = d.getPredecessor();
    if (dNext.isNull()) return;
    fmo::Pos pos = d.getCenter();
    fmo::Pos posNext = dNext.getCenter();
    auto dir = fmo::NormVector(posNext - pos);
    auto norm = fmo::perpendicular(dir);
    auto radius = d.getRadius();
    auto vA = shiftPoint(pos, radius, norm);
    auto vB = shiftPoint(pos, -radius, norm);
    color.a = DECAY_BASE + decay;
    b.addVertex(vA, color);
    b.addVertex(vA, color);
    b.addVertex(vB, color);

    while (true) {
        d = std::move(dNext);
        dNext = d.getPredecessor();
        pos = posNext;

        if (dNext.isNull()) break;

        posNext = dNext.getCenter();
        dir = fmo::NormVector(posNext - pos);
        auto normPrev = norm;
        norm = fmo::perpendicular(dir);
        auto normAvg = fmo::average(norm, normPrev);
        radius = d.getRadius();
        vA = shiftPoint(pos, radius, normAvg);
        vB = shiftPoint(pos, -radius, normAvg);
        color.a = DECAY_BASE + (decay *= DECAY_RATE);
        b.addVertex(vA, color);
        b.addVertex(vB, color);
        decay *= DECAY_RATE;
    }

    // last vertex
    radius = d.getRadius();
    vA = shiftPoint(pos, radius, norm);
    vB = shiftPoint(pos, -radius, norm);
    color.a = DECAY_BASE + (decay *= DECAY_RATE);
    b.addVertex(vA, color);
    b.addVertex(vB, color);
    b.addVertex(vB, color);
    decay *= DECAY_RATE;
}
