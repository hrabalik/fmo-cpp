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

    void log(log_t logFunc, const char* cStr) {
        logFunc(cStr);
    }

    template<typename Arg1, typename... Args>
    void log(log_t logFunc, const char* format, Arg1 arg1, Args... args) {
        char buf[81];
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wformat-security"
        snprintf(buf, sizeof(buf), format, arg1, args...);
#   pragma GCC diagnostic pop
        logFunc(buf);
    }

    struct {
        Reference<Callback> callbackRef;
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

void Registry::runAll(log_t logFunc, stop_t stopFunc) const {
    fmo::SectionStats stats;

    try {
        log(logFunc, "Benchmark started.\n");

        for (auto func : mFuncs) {
            stats.reset();
            bool updated = false;

            while (!updated && !stopFunc()) {
                stats.start();
                func.second();
                updated = stats.stop();
            }

            if (stopFunc()) {
                throw std::runtime_error("stopped");
            }

            auto q = stats.quantilesMs();
            log(logFunc, "%s: %.2f / %.1f / %.0f\n", func.first, q.q50, q.q95, q.q99);
        }

        log(logFunc, "Benchmark finished.\n\n");
    } catch (std::exception& e) {
        log(logFunc, "Benchmark interrupted: %s.\n\n", e.what());
    }
}

Benchmark::Benchmark(const char* name, bench_t func) {
    auto& reg = Registry::get();
    reg.add(name, func);
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
