#include "catch.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <fmo/image.hpp>
#include <fstream>

const char* const IM_4x2_FILE = "assets/4x2.png";

const fmo::Image::Dims IM_4x2_DIMS = {4, 2};

const std::array<uint8_t, 24> IM_4x2_BGR = {
    0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, // RGBC
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, // MYKW
};

const std::array<uint8_t, 8> IM_4x2_GRAY = {
    0x1D, 0x95, 0x4C, 0xB2, // RGBC gray
    0x69, 0xE1, 0x00, 0xFF, // MYKW gray
};

const std::array<uint8_t, 24> IM_4x2_GRAY_3 = {
    0x1D, 0x1D, 0x1D, 0x95, 0x95, 0x95, 0x4C, 0x4C, 0x4C, 0xB2, 0xB2, 0xB2, // RGBC gray 3x
    0x69, 0x69, 0x69, 0xE1, 0xE1, 0xE1, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, // MYKW gray 3x
};

const std::array<uint8_t, 12> IM_4x2_YUV420SP = {
    0x1D, 0x95, 0x4C, 0xB2, // RGBC gray
    0x69, 0xE1, 0x00, 0xFF, // MYKW gray
    0x80, 0x80, 0x80, 0x80, // UVUV gray
};

template <typename Lhs, typename Rhs>
bool exact_match(const Lhs& lhs, const Rhs& rhs) {
    auto res = std::mismatch(begin(lhs), end(lhs), begin(rhs), end(rhs));
    return res.first == end(lhs) && res.second == end(rhs);
}

template <typename Lhs, typename Rhs>
bool almost_exact_match(const Lhs& lhs, const Rhs& rhs) {
    auto res = std::mismatch(begin(lhs), end(lhs), begin(rhs), end(rhs), [](uint8_t l, uint8_t r) {
        return (std::max(l, r) - std::min(l, r)) <= 1;
    });
    return res.first == end(lhs) && res.second == end(rhs);
}

SCENARIO("reading images from files", "[image]") {
    WHEN("loading and converting a known image to BGR") {
        fmo::Image image{IM_4x2_FILE, fmo::Image::Format::BGR};

        THEN("image has correct dimensions") {
            REQUIRE(image.dims() == IM_4x2_DIMS);

            AND_THEN("image has correct format") {
                REQUIRE(image.format() == fmo::Image::Format::BGR);

                AND_THEN("image matches hard-coded values") {
                    REQUIRE(exact_match(image, IM_4x2_BGR));
                }
            }
        }
    }
    WHEN("loading and converting a known image to GRAY") {
        fmo::Image image{IM_4x2_FILE, fmo::Image::Format::GRAY};

        THEN("image has correct dimensions") {
            REQUIRE(image.dims() == IM_4x2_DIMS);

            AND_THEN("image has correct format") {
                REQUIRE(image.format() == fmo::Image::Format::GRAY);

                AND_THEN("image matches hard-coded values") {
                    REQUIRE(exact_match(image, IM_4x2_GRAY));
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

SCENARIO("performing color conversions", "[image]") {
    GIVEN("an empty destination image") {
        fmo::Image dest{};
        GIVEN("a BGR source image") {
            fmo::Image src{fmo::Image::Format::BGR, IM_4x2_DIMS, IM_4x2_BGR.data()};
            WHEN("converting to GRAY") {
                fmo::Image::convert(src, dest, fmo::Image::Format::GRAY);
                THEN("result image has correct dimensions") {
                    REQUIRE(dest.dims() == IM_4x2_DIMS);
                    AND_THEN("result image has correct format") {
                        REQUIRE(dest.format() == fmo::Image::Format::GRAY);
                        AND_THEN("result image matches hard-coded values") {
                            REQUIRE(almost_exact_match(dest, IM_4x2_GRAY));
                        }
                    }
                }
            }
        }
        GIVEN("a GRAY source image") {
            fmo::Image src{fmo::Image::Format::GRAY, IM_4x2_DIMS, IM_4x2_GRAY.data()};
            WHEN("converting to BGR") {
                fmo::Image::convert(src, dest, fmo::Image::Format::BGR);
                THEN("result image has correct dimensions") {
                    REQUIRE(dest.dims() == IM_4x2_DIMS);
                    AND_THEN("result image has correct format") {
                        REQUIRE(dest.format() == fmo::Image::Format::BGR);
                        AND_THEN("result image matches hard-coded values") {
                            REQUIRE(exact_match(dest, IM_4x2_GRAY_3));
                        }
                    }
                }
            }
        }
    }
}
