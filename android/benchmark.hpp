#include <vector>
#include <functional>

void benchLog(const char* cStr);

template<typename Arg1, typename... Args>
void benchLog(const char* format, Arg1 arg1, Args... args) {
    char buf[81];
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wformat-security"
    snprintf(buf, sizeof(buf), format, arg1, args...);
#   pragma GCC diagnostic pop
    benchLog(buf);
}

using bench_t = std::function<void()>;

struct Registry {
    Registry(const Registry&) = delete;

    Registry& operator=(const Registry&) = delete;

    static Registry& get();

    void add(const bench_t& func) { mFuncs.emplace_back(func); }

    void runAll() const;

private:
    Registry() = default;

    std::vector<bench_t> mFuncs;
};

struct Benchmark {
    Benchmark() = delete;

    Benchmark(const Benchmark&) = delete;

    Benchmark& operator=(const Benchmark&) = delete;

    Benchmark(const char* name, void (*func)());
};

#define FMO_CONCAT(a, b) a ## b
#define FMO_UNIQUE_NAME FMO_CONCAT(FMO_UNIQUE_NAME_, __COUNTER__)
