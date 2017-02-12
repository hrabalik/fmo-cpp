#include "catch.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <fmo/image.hpp>
#include <fstream>

const char* const IM_4x2_FILE = "assets/4x2.png";

const int IM_4x2_W = 4;
const int IM_4x2_H = 2;

const std::array<uint8_t, 24> IM_4x2_BGR = {
    0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, // RGBC
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, // MYKW
};

const std::array<uint8_t, 8> IM_4x2_GRAY = {
    0x1D, 0x95, 0x4C, 0xB2,
    0x69, 0xE1, 0x00, 0xFF,
};

const std::array<uint8_t, 12> IM_4x2_YUV420SP = {
    0x1D, 0x95, 0x4C, 0xB2,
    0x69, 0xE1, 0x00, 0xFF,
    0x80, 0x80, 0x80, 0x80,
};

SCENARIO("reading images from files", "[image]") {
    WHEN("loading and converting a known image to BGR") {
        fmo::Image image{IM_4x2_FILE, fmo::Image::Format::BGR};
        
        THEN("image has correct size") {
            auto size = image.size();
            REQUIRE(size.width == IM_4x2_W);
            REQUIRE(size.height == IM_4x2_H);

            AND_THEN("image has correct format") {
                REQUIRE(image.format() == fmo::Image::Format::BGR);

                AND_THEN("image matches hard-coded values") {
                    uint8_t* data = image.data();
                    auto& gt = IM_4x2_BGR;
                    auto res = std::mismatch(begin(gt), end(gt), data, data + gt.size());
                    REQUIRE(res.first == end(gt));
                }
            }
        }
    }
    WHEN("loading and converting a known image to GRAY") {
        fmo::Image image{IM_4x2_FILE, fmo::Image::Format::GRAY};

        THEN("image has correct size") {
            auto size = image.size();
            REQUIRE(size.width == IM_4x2_W);
            REQUIRE(size.height == IM_4x2_H);

            AND_THEN("image has correct format") {
                REQUIRE(image.format() == fmo::Image::Format::GRAY);
                
                AND_THEN("image matches hard-coded values") {
                    uint8_t* data = image.data();
                    auto& gt = IM_4x2_GRAY;
                    auto res = std::mismatch(begin(gt), end(gt), data, data + gt.size());
                    REQUIRE(res.first == end(gt));
                }
            }
        }
    }
    WHEN("loading into an unsupported format") {
        THEN("constructor throws") {
            REQUIRE_THROWS(fmo::Image(IM_4x2_FILE, fmo::Image::Format::UNKNOWN));
        }
    }
    WHEN("the image file doesn't exist") {
        THEN("constructor throws") {
            REQUIRE_THROWS(fmo::Image("Eh3qUrSOFl", fmo::Image::Format::BGR));
        }
    }
}
