#include "benchmark.hpp"
#include "java_classes.hpp"
#include "java_interface.hpp"
#include <fmo/stats.hpp>
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
        fmo::SectionStats stats;
        JavaVM* javaVM;
        JNIEnv* threadEnv;
        std::atomic<bool> stop;
    } global;
}

void benchLog(const char* cStr) {
    Callback cb{global.callbackRef.get(global.threadEnv)};
    cb.log(cStr);
}


Registry& Registry::get() {
    static Registry instance;
    return instance;
}

void Registry::runAll() const { for (auto& func : mFuncs) func(); }

Benchmark::Benchmark(const char* name, void (*func)()) {
    auto& reg = Registry::get();

    reg.add([name, func]() {
        global.stats.reset();
        bool updated = false;

        while (!updated && !global.stop) {
            global.stats.start();
            func();
            updated = global.stats.stop();
        }

        if (global.stop) {
            throw std::runtime_error("stopped");
        }

        auto q = global.stats.quantilesMs();
        benchLog("%s: %.2f / %.1f / %.0f\n", name, q.q50, q.q95, q.q99);
    });
}

void Java_cz_fmo_Lib_benchmarkingStart(JNIEnv* env, jclass, jobject cbObj) {
    global.callbackRef = {env, cbObj};
    global.stop = false;
    env->GetJavaVM(&global.javaVM);

    std::thread thread([]() {
        Env threadEnv{global.javaVM, "benchmarking"};
        global.threadEnv = threadEnv.get();

        benchLog("Benchmark started.\n");

        try {
            Registry::get().runAll();
        } catch (std::exception& e) {
            benchLog("Benchmark interrupted: %s\n\n", e.what());
            return;
        }

        benchLog("Benchmark finished.\n\n");
    });

    thread.detach();
}

void Java_cz_fmo_Lib_benchmarkingStop(JNIEnv*, jclass) {
    global.stop = true;
}
