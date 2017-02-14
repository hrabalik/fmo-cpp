#include "java_classes.hpp"
#include "java_interface.hpp"
#include <fmo/stats.hpp>
#include <fmo/image.hpp>

namespace {
    //const auto INPUT_FORMAT = fmo::Format::YUV420SP;

    struct {
        Reference<Callback> callbackRef;
        fmo::FrameStats frameStats;
        fmo::SectionStats sectionStats;
        std::vector<uint8_t> buffer;
        bool statsUpdated;
        fmo::Image image;
        fmo::Image image2;
        fmo::Dims dims;
    } global;
}

void Java_cz_fmo_Lib_recording2Start(JNIEnv* env, jclass, jint width, jint height, jobject cbObj) {
    global.callbackRef = {env, cbObj};
    global.frameStats.reset(30.f);
    global.sectionStats.reset();
    global.statsUpdated = true;
    global.dims = {width, height};
}

void Java_cz_fmo_Lib_recording2Stop(JNIEnv* env, jclass) { global.callbackRef.release(env); }

void Java_cz_fmo_Lib_recording2Frame(JNIEnv* env, jclass, jbyteArray dataYUV420SP) {
    if (global.statsUpdated) {
        global.statsUpdated = false;
        //auto q = global.frameStats.quantilesHz();
        auto q = global.sectionStats.quantilesMs();
        auto callback = global.callbackRef.get(env);
        callback.frameTimings(q.q50, q.q95, q.q99);
    }

    global.frameStats.tick();

    jbyte* ptr = env->GetByteArrayElements(dataYUV420SP, nullptr);
    auto dataPtr = reinterpret_cast<uint8_t*>(ptr);
    global.image.assign(fmo::Format::GRAY, global.dims, dataPtr);
    global.sectionStats.start();
    fmo::less_than(global.image, global.image, 0x80);
    global.statsUpdated = global.sectionStats.stop();
    env->ReleaseByteArrayElements(dataYUV420SP, ptr, JNI_ABORT);
}
