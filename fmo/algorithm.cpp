#include <fmo/algorithm.hpp>
#include <map>

namespace fmo {
    Algorithm::Config::Config(std::string aName, Format aFormat, Dims aDims)
        : name(std::move(aName)),
          format(aFormat),
          dims(aDims),
          diff(),
          minGap(0.10f),
          maxImageHeight(300),
          minStripHeight(2),
          minStripsInComponent(2),
          minStripsInCluster(12),
          heightRatioWeight(1.f),
          distanceWeight(1.f),
          maxHeightRatioInternal(1.51f),
          maxHeightRatioExternal(1.51f),
          maxDistance(10.f),
          minMotion(0.25),
          objectResolution(PROCESSING) {}

    using AlgorithmRegistry = std::map<std::string, Algorithm::Factory>;

    AlgorithmRegistry& getRegistry() {
        static AlgorithmRegistry registry;
        return registry;
    }

    void registerExplorerV1();
    void registerExplorerV2();

    void registerBuiltInFactories() {
        static bool registered = false;
        if (registered) return;
        registered = true;

        registerExplorerV1();
        registerExplorerV2();
    }

    const std::string& fmo::Algorithm::defaultName() {
        static std::string result = "explorer-v2";
        return result;
    }

    std::unique_ptr<Algorithm> fmo::Algorithm::make(const Config& config) {
        registerBuiltInFactories();
        auto& registry = getRegistry();
        auto it = registry.find(config.name);
        if (it == registry.end()) { throw std::runtime_error("unknown algorithm name"); }
        return it->second(config);
    }

    void Algorithm::registerFactory(const std::string& name, const Factory& factory) {
        auto& registry = getRegistry();
        auto it = registry.find(name);
        if (it != registry.end()) { throw std::runtime_error("duplicate algorithm name"); }
        registry.emplace(name, factory);
    }
}
