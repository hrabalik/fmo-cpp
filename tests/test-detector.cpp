#include "../catch/catch.hpp"
#include <fmo/detector2.hpp>
#include <fmo/processing.hpp>
#include <string>
#include <array>

namespace {
    const std::array<const char*, 8> FILES = {
        "assets/seq2_1_gray.jpg",
        "assets/seq2_3_gray.jpg",
        "assets/seq3_1_gray.jpg",
        "assets/seq3_3_gray.jpg",
        "assets/seq4_1_gray.jpg",
        "assets/seq4_3_gray.jpg",
        "assets/seq5_1_gray.jpg",
        "assets/seq5_3_gray.jpg",
    };
}

SCENARIO("running the detector on large files", "[detector]") {
    GIVEN("two large grayscale images") {
        std::array<fmo::Image, FILES.size()> src;
        for (int i = 0; i < FILES.size(); i++) {
            src[i] = fmo::Image{FILES[i], fmo::Format::GRAY};
        }
        WHEN("the absolute difference is calculated") {
            std::array<fmo::Image, FILES.size() / 2> diff;
            for (int i = 0; i < FILES.size(); i += 2) {
                fmo::absdiff(src[i], src[i + 1], diff[i / 2]);
            }
            AND_WHEN("detector is run on the difference image") {
                fmo::Detector2::Config cfg;
                cfg.dims = fmo::Dims{1920, 1080};
                fmo::Detector2 detector(cfg);
                for (int i = 0; i < FILES.size() / 2; i++) {
                    detector.setInput(diff[i]);
                    auto& vis = detector.getDebugImage();
                    std::string file = "temp_detect2_" + std::to_string(i) + ".png";
                    fmo::save(vis, file);
                }
            }
        }
    }
}
