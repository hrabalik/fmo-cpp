#include <fmo/algorithm.hpp>
#include <map>

namespace fmo {
    Algorithm::Config::Config()
        : name("explorer-v2"),
          diff(),
          minGap(0.05f),
          maxImageHeight(300),
          minStripHeight(2),
          minStripsInComponent(2),
          minStripsInCluster(12),
          heightRatioWeight(1.f),
          distanceWeight(0.f),
          gapsWeight(1.f),
          maxHeightRatioStrips(1.75001f),
          maxHeightRatioInternal(1.75001f),
          maxHeightRatioExternal(1.99999f),
          maxDistance(20.f),
          maxGapsLength(0.75f),
          minMotion(0.25f),
          maxMotion(0.50f),
          pointSetSourceResolution(false) {}

    using AlgorithmRegistry = std::map<std::string, Algorithm::Factory>;

    AlgorithmRegistry& getRegistry() {
        static AlgorithmRegistry registry;
        return registry;
    }

    void registerExplorerV1();
    void registerExplorerV2();
    void registerExplorerV3();

    void registerBuiltInFactories() {
        static bool registered = false;
        if (registered) return;
        registered = true;

        registerExplorerV1();
        registerExplorerV2();
        registerExplorerV3();
    }

    std::unique_ptr<Algorithm> fmo::Algorithm::make(const Config& config, Format format,
                                                    Dims dims) {
        registerBuiltInFactories();
        auto& registry = getRegistry();
        auto it = registry.find(config.name);
        if (it == registry.end()) { throw std::runtime_error("unknown algorithm name"); }
        return it->second(config, format, dims);
    }

    void Algorithm::registerFactory(const std::string& name, const Factory& factory) {
        auto& registry = getRegistry();
        auto it = registry.find(name);
        if (it != registry.end()) { throw std::runtime_error("duplicate algorithm name"); }
        registry.emplace(name, factory);
    }

    std::vector<std::string> Algorithm::listFactories() {
        registerBuiltInFactories();
        const auto& registry = getRegistry();
        std::vector<std::string> result;

        for (auto& entry : registry) {
            result.push_back(entry.first);
        }

        return result;
    }
}
