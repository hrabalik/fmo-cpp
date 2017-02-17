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

    template<typename... Args>
    void log(const char* format, Args... args) {
        char buf[80];
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wformat-security"
        snprintf(buf, sizeof(buf), format, args...);
#   pragma GCC diagnostic pop
        Callback cb{global.callbackRef.get(global.threadEnv)};
        cb.log(buf);
    }

    template<typename Func>
    void run(const char* name, Func func) {
        global.stats.reset();
        bool updated = false;

        while (!updated && !global.stop) {
            global.stats.start();

            updated = global.stats.stop();
        }

        if (global.stop) {
            throw std::runtime_error("benchmarking interrupted");
        }

        auto q = global.stats.quantilesMs();
        log("%s: %.2f / %.1f / %.0f\n", name, q.q50, q.q95, q.q99);
    }
}

void Java_cz_fmo_Lib_benchmarkingStart(JNIEnv* env, jclass, jobject cbObj) {
    global.callbackRef = {env, cbObj};
    global.stop = false;
    env->GetJavaVM(&global.javaVM);

    std::thread thread([]() {
        Env threadEnv{global.javaVM, "benchmarking"};
        global.threadEnv = threadEnv.get();

        log("Benchmark started.\n");
        auto status = runCatch();

        if (status == BenchResult::GOOD) {
            log("Benchmark finished.\n\n");
        } else {
            log("Benchmark failed.\n\n");
        }
    });

    thread.detach();
}

void Java_cz_fmo_Lib_benchmarkingStop(JNIEnv*, jclass) {
    global.stop = true;
}

TEST_CASE("stuff") {
    run(__func__, []() {

    });
}
