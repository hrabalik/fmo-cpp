#include "catch.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <fmo/image.hpp>
#include <fstream>

const char* const IM_4x2_FILE = "assets/4x2.png";

const fmo::Dims IM_4x2_DIMS = {4, 2};

const std::array<uint8_t, 24> IM_4x2_BGR = {
    0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, // RGBC
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, // MYKW
};

const std::array<uint8_t, 6> IM_2x1_BGR = {
    0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00 // YK
};

const std::array<uint8_t, 8> IM_4x2_GRAY = {
    0x1D, 0x95, 0x4C, 0xB2, // RGBC gray
    0x69, 0xE1, 0x00, 0xFF, // MYKW gray
};

const std::array<uint8_t, 4> IM_2x2_GRAY = {
    0x95, 0x4C, // GB gray
    0xE1, 0x00, // YK gray
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

// YUV-to-RGB conversion is slightly more involved than just assuming that the Y value is
// brightness -- the Y value is assumed to be between 16 and 235.
const std::array<uint8_t, 24> IM_4x2_YUV2BGR = {
    0x0F, 0x0F, 0x0F, 0x9B, 0x9B, 0x9B, 0x46, 0x46, 0x46, 0xBD, 0xBD, 0xBD,
    0x68, 0x68, 0x68, 0xF3, 0xF3, 0xF3, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
};

template <typename Lhs, typename Rhs>
bool exact_match(const Lhs& lhs, const Rhs& rhs) {
    auto res = std::mismatch(begin(lhs), end(lhs), begin(rhs), end(rhs));
    return res.first == end(lhs) && res.second == end(rhs);
}

template <typename Lhs, typename Rhs>
bool almost_exact_match(const Lhs& lhs, const Rhs& rhs, uint8_t maxError) {
    auto res = std::mismatch(begin(lhs), end(lhs), begin(rhs), end(rhs), [=](uint8_t l, uint8_t r) {
        return ((l > r) ? (l - r) : (r - l)) <= maxError;
    });
    return res.first == end(lhs) && res.second == end(rhs);
}

SCENARIO("reading images from files", "[image]") {
    WHEN("loading and converting a known image to BGR") {
        fmo::Image image{IM_4x2_FILE, fmo::Format::BGR};
        THEN("image has correct dimensions") {
            REQUIRE(image.dims() == IM_4x2_DIMS);
            AND_THEN("image has correct format") {
                REQUIRE(image.format() == fmo::Format::BGR);
                AND_THEN("image matches hard-coded values") {
                    REQUIRE(exact_match(image, IM_4x2_BGR));
                }
            }
        }
    }
    WHEN("loading and converting a known image to GRAY") {
        fmo::Image image{IM_4x2_FILE, fmo::Format::GRAY};
        THEN("image has correct dimensions") {
            REQUIRE(image.dims() == IM_4x2_DIMS);
            AND_THEN("image has correct format") {
                REQUIRE(image.format() == fmo::Format::GRAY);
                AND_THEN("image matches hard-coded values") {
                    REQUIRE(exact_match(image, IM_4x2_GRAY));
                }
            }
        }
    }
    WHEN("loading into an unsupported format") {
        THEN("constructor throws") {
            REQUIRE_THROWS(fmo::Image(IM_4x2_FILE, fmo::Format::UNKNOWN));
        }
    }
    WHEN("the image file doesn't exist") {
        THEN("constructor throws") {
            REQUIRE_THROWS(fmo::Image("Eh3qUrSOFl", fmo::Format::BGR));
        }
    }
}

SCENARIO("performing color conversions", "[image]") {
    GIVEN("a BGR source image") {
        fmo::Image src{fmo::Format::BGR, IM_4x2_DIMS, IM_4x2_BGR.data()};
        GIVEN("an empty destination image") {
            fmo::Image dst{};
            WHEN("converting to GRAY") {
                fmo::convert(src, dst, fmo::Format::GRAY);
                THEN("result image has correct dimensions") {
                    REQUIRE(dst.dims() == IM_4x2_DIMS);
                    AND_THEN("result image has correct format") {
                        REQUIRE(dst.format() == fmo::Format::GRAY);
                        AND_THEN("result image matches hard-coded values") {
                            REQUIRE(almost_exact_match(dst, IM_4x2_GRAY, 1));
                        }
                    }
                }
            }
        }
    }
    GIVEN("a GRAY source image") {
        fmo::Image src{fmo::Format::GRAY, IM_4x2_DIMS, IM_4x2_GRAY.data()};
        GIVEN("an empty destination image") {
            fmo::Image dst{};
            WHEN("converting to BGR") {
                fmo::convert(src, dst, fmo::Format::BGR);
                THEN("result image has correct dimensions") {
                    REQUIRE(dst.dims() == IM_4x2_DIMS);
                    AND_THEN("result image has correct format") {
                        REQUIRE(dst.format() == fmo::Format::BGR);
                        AND_THEN("result image matches hard-coded values") {
                            REQUIRE(exact_match(dst, IM_4x2_GRAY_3));
                        }
                    }
                }
            }
        }
    }
    GIVEN("a YUV420 source image") {
        fmo::Image src{fmo::Format::YUV420SP, IM_4x2_DIMS, IM_4x2_YUV420SP.data()};
        GIVEN("an empty destination image") {
            fmo::Image dst{};
            WHEN("converting to GRAY") {
                fmo::convert(src, dst, fmo::Format::GRAY);
                THEN("result image has correct dimensions") {
                    REQUIRE(dst.dims() == IM_4x2_DIMS);
                    AND_THEN("result image has correct format") {
                        REQUIRE(dst.format() == fmo::Format::GRAY);
                        AND_THEN("result image matches hard-coded values") {
                            REQUIRE(exact_match(dst, IM_4x2_GRAY));
                        }
                    }
                }
            }
            WHEN("converting to BGR") {
                fmo::convert(src, dst, fmo::Format::BGR);
                THEN("result image has correct dimensions") {
                    REQUIRE(dst.dims() == IM_4x2_DIMS);
                    AND_THEN("result image has correct format") {
                        REQUIRE(dst.format() == fmo::Format::BGR);
                        AND_THEN("result image matches hard-coded values") {
                            REQUIRE(exact_match(dst, IM_4x2_YUV2BGR));
                        }
                    }
                }
            }
        }
        GIVEN("converting to GRAY in-place") {
            auto* dataPtrBefore = src.data();
            fmo::convert(src, src, fmo::Format::GRAY);
            THEN("image has correct dimensions") {
                REQUIRE(src.dims() == IM_4x2_DIMS);
                AND_THEN("image has correct format") {
                    REQUIRE(src.format() == fmo::Format::GRAY);
                    AND_THEN("image matches hard-coded values") {
                        REQUIRE(exact_match(src, IM_4x2_GRAY));
                        AND_THEN("there was no allocation") {
                            auto* dataPtrAfter = src.data();
                            REQUIRE(dataPtrBefore == dataPtrAfter);
                        }
                    }
                }
            }
        }
    }
}

SCENARIO("extracting regions from images") {
    GIVEN("a BGR source image") {
        fmo::Image src{fmo::Format::BGR, IM_4x2_DIMS, IM_4x2_BGR.data()};
        GIVEN("an empty destination image") {
            fmo::Image dst{ };
            WHEN("a 2x1 region is created from source image") {
                const fmo::Pos pos{1, 1};
                const fmo::Dims dims{2, 1};
                fmo::Region reg = src.region(pos, dims);
                THEN("region has correct position, dimensions and format") {
                    REQUIRE(reg.pos() == pos);
                    REQUIRE(reg.dims() == dims);
                    REQUIRE(reg.format() == fmo::Format::BGR);
                    AND_THEN("region data points to the expected location") {
                        auto* data = src.data();
                        data += 3 * pos.x;
                        data += 3 * src.dims().width * pos.y;
                        REQUIRE(reg.data() == data);
                        WHEN("region is copied to destination image") {
                            fmo::copy(reg, dst);
                            THEN("image has correct dimensions and format") {
                                REQUIRE(dst.dims() == dims);
                                REQUIRE(dst.format() == fmo::Format::BGR);
                                AND_THEN("image contains expected data") {
                                    REQUIRE(exact_match(dst, IM_2x1_BGR));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    GIVEN("a GRAY source image") {
        fmo::Image src{fmo::Format::GRAY, IM_4x2_DIMS, IM_4x2_GRAY.data()};
        GIVEN("an empty destination image") {
            fmo::Image dst{ };
            WHEN("a 2x2 region is created from source image") {
                const fmo::Pos pos{1, 0};
                const fmo::Dims dims{2, 2};
                fmo::Region reg = src.region(pos, dims);
                THEN("region has correct position, dimensions and format") {
                    REQUIRE(reg.pos() == pos);
                    REQUIRE(reg.dims() == dims);
                    REQUIRE(reg.format() == fmo::Format::GRAY);
                    AND_THEN("region data points to the expected location") {
                        auto* data = src.data();
                        data += 1 * pos.x;
                        data += 1 * src.dims().width * pos.y;
                        REQUIRE(reg.data() == data);
                        WHEN("region is copied to destination image") {
                            fmo::copy(reg, dst);
                            THEN("image has correct dimensions and format") {
                                REQUIRE(dst.dims() == dims);
                                REQUIRE(dst.format() == fmo::Format::GRAY);
                                AND_THEN("image contains expected data") {
                                    REQUIRE(exact_match(dst, IM_2x2_GRAY));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
