#include <fmo/benchmark.hpp>
#include "java_classes.hpp"
#include "java_interface.hpp"
#include <thread>
#include <atomic>

namespace {
    struct Env {
        Env(const Env&) = delete;

        Env& operator=(const Env&) = delete;

        Env(JavaVM* vm, const char* threadName) : mVM(vm) {
            JavaVMAttachArgs args = {JNI_VERSION_1_6, threadName, nullptr};
            jint result = mVM->AttachCurrentThread(&mPtr, &args);
            FMO_ASSERT(result == JNI_OK, "AttachCurrentThread failed");
        }

        ~Env() { mVM->DetachCurrentThread(); }

        JNIEnv* get() { return mPtr; }

        JNIEnv& operator->() { return *mPtr; }

    private:
        JavaVM* mVM;
        JNIEnv* mPtr = nullptr;
    };

    struct {
        Reference<Callback> callbackRef;
        JavaVM* javaVM;
        JNIEnv* threadEnv;
        std::atomic<bool> stop;
    } global;
}

void Java_cz_fmo_Lib_benchmarkingStart(JNIEnv* env, jclass, jobject cbObj) {
    global.callbackRef = {env, cbObj};
    global.stop = false;
    env->GetJavaVM(&global.javaVM);

    std::thread thread([]() {
        Env threadEnv{global.javaVM, "benchmarking"};
        global.threadEnv = threadEnv.get();
        Registry::get().runAll([](const char* cStr) {
                                   Callback cb{global.callbackRef.get(global.threadEnv)};
                                   cb.log(cStr);
                               },
                               []() {
                                   return bool(global.stop);
                               });
    });

    thread.detach();
}

void Java_cz_fmo_Lib_benchmarkingStop(JNIEnv*, jclass) {
    global.stop = true;
}
