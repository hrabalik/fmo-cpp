#include <fmo/benchmark.hpp>
#include <fmo/stats.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <random>

namespace {
    void log(log_t logFunc, const char* cStr) { logFunc(cStr); }

    template <typename Arg1, typename... Args>
    void log(log_t logFunc, const char* format, Arg1 arg1, Args... args) {
        char buf[81];
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
        snprintf(buf, sizeof(buf), format, arg1, args...);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
        logFunc(buf);
    }
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

            if (stopFunc()) { throw std::runtime_error("stopped"); }

            auto q = stats.quantilesMs();
            log(logFunc, "%s: %.2f / %.1f / %.0f\n", func.first, q.q50, q.q95, q.q99);
        }

        log(logFunc, "Benchmark finished.\n\n");
    } catch (std::exception& e) { log(logFunc, "Benchmark interrupted: %s.\n\n", e.what()); }
}

Benchmark::Benchmark(const char* name, bench_t func) {
    auto& reg = Registry::get();
    reg.add(name, func);
}

namespace {
    struct {
        cv::Mat grayNoise;
        cv::Mat grayCheckerboard;
        cv::Mat out1;
        cv::Mat out2;
        cv::Mat out3;
    } global;

    struct Init {
        static const int W = 1920;
        static const int H = 1080;

        static cv::Mat newGrayMat() { return {cv::Size{W, H}, CV_8UC1}; }

        Init() {
            std::mt19937 re{5489};
            using limits = std::numeric_limits<int>;
            std::uniform_int_distribution<int> uniform{limits::min(), limits::max()};

            {
                global.grayNoise = newGrayMat();
                auto* data = global.grayNoise.data;
                auto* end = data + (W * H);

                for (; data < end; data += sizeof(int)) { *(int*)data = uniform(re); }

                // cv::imwrite("grayNoise.png", global.grayNoise);
            }

            {
                global.grayCheckerboard = newGrayMat();
                auto* data = global.grayCheckerboard.data;

                for (int r = 0; r < H; r++) {
                    bool a = ((r*r) % 100000) > 50000;
                    for (int c = 0; c < W; c++) {
                        bool b = (c % 512) > 256;
                        *data++ = (a ^ b) ? 0xFF : 0x00;
                    }
                }

                //cv::imwrite("grayCheckerboard.png", global.grayCheckerboard);
            }
        }
    };

    void init() { static Init once; }

    Benchmark FMO_UNIQUE_NAME{"threshold", []() {
                                  init();
                                  cv::threshold(global.grayNoise, global.out1, 0x80, 0xFF,
                                                cv::THRESH_BINARY);
                              }};

    Benchmark FMO_UNIQUE_NAME{"connectedComponents", []() {
                                  init();
                                  cv::connectedComponentsWithStats(global.grayCheckerboard, global.out1,
                                                                   global.out2, global.out3);
                              }};
}
