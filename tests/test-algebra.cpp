#include "../catch/catch.hpp"
#include <fmo/algebra.hpp>

using namespace fmo;

constexpr Vector v1{303, 679, 427};
constexpr Vector v2{453, 501, 116};
constexpr EPoint p1{219, 627};
constexpr EPoint p2{103, 547};
static_assert(same_vector(cross(v1, v2), {-135163, 158283, -155784}), "cross()");
static_assert(same_vector(cross(v1, p2), {-232890, 43678, 95804}), "cross()");
static_assert(same_vector(cross(p1, v2), {72231, -24951, -174312}), "cross()");
static_assert(same_vector(cross(p1, p2), {80, -116, 55212}), "cross()");
static_assert(same_scalar(dot(v1, v2), 526970), "dot()");
static_assert(same_scalar(dot(v1, p2), 403049), "dot()");
static_assert(same_scalar(dot(p1, v2), 413450), "dot()");
static_assert(same_scalar(dot(p1, p2), 365527), "dot()");
constexpr NLine l1{0.998612f, 0.052671f, 8.432412};
constexpr NLine l2{0.540302f, -0.841471f, -6.022023};

TEST_CASE("euclidean()", "[algebra]") {
    float a = -6.23049f;
    float b = 0.23344f;
    Point p1p{a * p1.x, a * p1.y, a};
    Point p2p{b * p2.x, b * p2.y, b};
    REQUIRE(same_vector(euclidean(p1p), p1, 1e-4f));
    REQUIRE(same_vector(euclidean(p2p), p2, 1e-4f));
}

TEST_CASE("normalize()", "[algebra]") {
    float a = 6.23049f;
    float b = 0.02334f;
    Line l1d{a * l1.x, a * l1.y, a * l1.w};
    Line l2d{b * l2.x, b * l2.y, b * l2.w};
    REQUIRE(same_vector(normalize(l1d), l1, 1e-4f));
    REQUIRE(same_vector(normalize(l2d), l2, 1e-4f));
}
