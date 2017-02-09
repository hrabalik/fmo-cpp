#include "catch.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <fmo/image.hpp>
#include <fstream>

const std::array<uint8_t, 18> IM_3x2_BGR = {
    0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
};

const std::array<uint8_t, 6> IM_3x2_GRAY = {
    0x1D, 0x95, 0x4C,
    0x00, 0xFF, 0x00,
};

TEST_CASE("reading images from files", "[image]") {
    WHEN("loading and converting a known image to BGR") {
        THEN("image matches hard-coded values") {
            fmo::Image image{"assets/3x2.png", fmo::Image::Format::BGR};
            uint8_t* data = image.data();
            auto& gt = IM_3x2_BGR;
            auto res = std::mismatch(begin(gt), end(gt), data, data + gt.size());
            REQUIRE(res.first == end(gt));
        }
    }
    WHEN("loading and converting a known image to GRAY") {
        THEN("image matches hard-coded values") {
            fmo::Image image{"assets/3x2.png", fmo::Image::Format::GRAY};
            uint8_t* data = image.data();
            auto& gt = IM_3x2_GRAY;
            auto res = std::mismatch(begin(gt), end(gt), data, data + gt.size());
            REQUIRE(res.first == end(gt));
        }
    }
    WHEN("the image file doesn't exist") {
        THEN("constructor throws") {
            REQUIRE_THROWS(fmo::Image("Eh3qUrSOFl", fmo::Image::Format::BGR));
        }
    }
}
