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
        fmo::Image image1;
        fmo::Image image2;
        fmo::Image image3;
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
    global.image1.swap(global.image2);
    global.image2.resize(fmo::Format::YUV420SP, global.dims);
    global.image1.assign(fmo::Format::YUV420SP, global.dims, dataPtr);
    global.sectionStats.start();
    fmo::absdiff(global.image1, global.image2, global.image3);
    global.statsUpdated = global.sectionStats.stop();
    env->ReleaseByteArrayElements(dataYUV420SP, ptr, JNI_ABORT);
}
