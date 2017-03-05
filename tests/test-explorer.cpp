#include "../catch/catch.hpp"
#include <array>
#include <fmo/explorer.hpp>
#include <fmo/processing.hpp>
#include <string>

namespace {
    const std::array<const char*, 8> FILES = {{
        "assets/seq2_1_gray.jpg", "assets/seq2_3_gray.jpg", // table tennis hard
        "assets/seq3_1_gray.jpg", "assets/seq3_3_gray.jpg", // table tennis easy
        "assets/seq4_1_gray.jpg", "assets/seq4_3_gray.jpg", // table tennis orange ball
        "assets/seq5_1_gray.jpg", "assets/seq5_3_gray.jpg", // tennis serve
    }};
}

SCENARIO("running the explorer on large files", "[explorer]") {
    GIVEN("two large grayscale images") {
        std::array<fmo::Image, FILES.size()> src;
        for (size_t i = 0; i < FILES.size(); i++) {
            src[i] = fmo::Image{FILES[i], fmo::Format::GRAY};
        }
        WHEN("the absolute difference is calculated") {
            std::array<fmo::Image, FILES.size() / 2> diff;
            for (size_t i = 0; i < FILES.size(); i += 2) {
                fmo::absdiff(src[i], src[i + 1], diff[i / 2]);
            }
            AND_WHEN("explorer is run on the difference image") {
                fmo::Explorer::Config cfg;
                cfg.dims = fmo::Dims{1920, 1080};
                fmo::Explorer explorer(cfg);
                for (size_t i = 0; i < FILES.size() / 2; i++) {
                    explorer.setInput(diff[i]);
                    auto& vis = explorer.getDebugImage();
                    std::string file = "temp_detect2_" + std::to_string(i) + ".png";
                    fmo::save(vis, file);
                }
            }
        }
    }
}
