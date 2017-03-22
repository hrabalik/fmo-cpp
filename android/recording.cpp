#include "env.hpp"
#include "java_classes.hpp"
#include <atomic>
#include <fmo/algorithm.hpp>
#include <fmo/exchange.hpp>
#include <fmo/processing.hpp>
#include <fmo/stats.hpp>
#include <iomanip>
#include <sstream>
#include <thread>

namespace {
    constexpr fmo::Format INPUT_FORMAT = fmo::Format::GRAY;

    struct {
        std::mutex mutex;
        JavaVM* javaVM;
        std::atomic<bool> stop;
        std::unique_ptr<fmo::Exchange<fmo::Image>> exchange;
        Reference<Callback> callbackRef;
        fmo::Image image;
        fmo::Dims dims;
    } global;

    std::string statsString(const fmo::SectionStats& stats) {
        auto q = stats.quantilesMs();
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << q.q50 << " / ";
        oss << std::fixed << std::setprecision(1) << q.q95 << " / ";
        oss << std::fixed << std::setprecision(0) << q.q99;
        return oss.str();
    }

    void threadImpl() {
        Env threadEnv{global.javaVM, "recording"};
        JNIEnv* env = threadEnv.get();
        fmo::FrameStats frameStats;
        frameStats.reset(30);
        fmo::SectionStats sectionStats;
        fmo::Image input{INPUT_FORMAT, global.dims};
        fmo::Algorithm::Config config{fmo::Algorithm::defaultName(), fmo::Format::GRAY,
                                      global.dims};
        auto explorer = fmo::Algorithm::make(config);
        Callback callback = global.callbackRef.get(env);
        callback.log("Detection started");

        while (!global.stop) {
            global.exchange->swapReceive(input);
            if (global.stop) break;

            frameStats.tick();
            sectionStats.start();
            explorer->setInputSwap(input);
            bool statsUpdated = sectionStats.stop();

            if (statsUpdated) {
                std::string stats = statsString(sectionStats);
                callback.log(stats.c_str());
            }
        }

        global.callbackRef.release(env);
    }

    bool running() { return bool(global.exchange); }
}

void Java_cz_fmo_Lib_detectionStart(JNIEnv* env, jclass, jint width, jint height, jobject cbObj) {
    std::unique_lock<std::mutex> lock(global.mutex);
    global.dims = {width, height};
    env->GetJavaVM(&global.javaVM);
    global.stop = false;
    global.exchange.reset(new fmo::Exchange<fmo::Image>(INPUT_FORMAT, global.dims));
    global.callbackRef = {env, cbObj};

    std::thread thread(threadImpl);
    thread.detach();

    global.image.resize(INPUT_FORMAT, global.dims);
}

void Java_cz_fmo_Lib_detectionStop(JNIEnv* env, jclass) {
    std::unique_lock<std::mutex> lock(global.mutex);
    if (!running()) return;
    global.stop = true;
    global.exchange->exit();
}

void Java_cz_fmo_Lib_detectionFrame(JNIEnv* env, jclass, jbyteArray dataYUV420SP) {
    std::unique_lock<std::mutex> lock(global.mutex);
    if (!running()) return;
    jbyte* dataJ = env->GetByteArrayElements(dataYUV420SP, nullptr);
    uint8_t* data = reinterpret_cast<uint8_t*>(dataJ);
    global.image.assign(INPUT_FORMAT, global.dims, data);
    global.exchange->swapSend(global.image);
    env->ReleaseByteArrayElements(dataYUV420SP, dataJ, JNI_ABORT);
}
