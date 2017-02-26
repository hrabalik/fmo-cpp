#include "../catch/catch.hpp"
#include <fmo/detector.hpp>
#include <fmo/processing.hpp>
#include <string>

const char* const IM_LARGE1 = "assets/seq2_1_gray.jpg";
const char* const IM_LARGE2 = "assets/seq2_3_gray.jpg";

SCENARIO("running the detector on large files", "[detector]") {
    GIVEN("two large grayscale images") {
        fmo::Image src1{IM_LARGE1, fmo::Format::GRAY};
        fmo::Image src2{IM_LARGE2, fmo::Format::GRAY};
        WHEN("the absolute difference is calculated") {
            fmo::Image diff;
            fmo::absdiff(src1, src2, diff);
            AND_WHEN("detector is run on the difference image") {
                fmo::Detector detector;
                detector.setInput(diff);
                THEN("print the debug images to files") {
                    auto& vis = detector.getDebugImages();

                    for (int i = 0; i < vis.size(); i++) {
                        std::string file = "tmp_detect_" + std::to_string(i) + ".png";
                        fmo::save(vis[i], file);
                    }
                }
            }
        }
    }
}
