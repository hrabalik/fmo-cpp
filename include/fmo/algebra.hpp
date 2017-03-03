#ifndef FMO_ALGEBRA_HPP
#define FMO_ALGEBRA_HPP

#include <cmath>

namespace fmo {
    struct EPoint;
    struct Vector;
    struct Line;

    /// Vector in projective coordinates.
    struct Vector {
        constexpr Vector(float aX, float aY, float aW) : x(aX), y(aY), w(aW) {}

        Vector(const Vector&) = default;
        Vector& operator=(const Vector&) = default;

        // data
        float x, y, w;
    };

    /// Point in projective coordinates.
    struct Point : public Vector {
        constexpr Point(float aX, float aY, float aW) : Vector(aX, aY, aW) {}
        constexpr explicit Point(const Vector& v) : Vector(v) {}

        Point(const Point&) = default;
        Point& operator=(const Point&) = default;
    };

    /// Line in projective coordinates.
    struct Line : public Vector {
        constexpr Line(float aX, float aY, float aW) : Vector(aX, aY, aW) {}
        constexpr explicit Line(const Vector& v) : Vector(v) {}

        Line(const Line&) = default;
        Line& operator=(const Line&) = default;
    };

    /// Euclidean point -- point with a unit homogeneous coordinate.
    struct EPoint : public Point {
        constexpr EPoint(float aX, float aY) : Point(aX, aY, 1) {}

        EPoint(const EPoint&) = default;
        EPoint& operator=(const EPoint&) = default;
    };

    /// Normalized line -- line with a unit normal.
    struct NLine : public Line {
        constexpr NLine(float aX, float aY, float aW) : Line(aX, aY, aW) {}

        NLine(const NLine&) = default;
        NLine& operator=(const NLine&) = default;
    };

    /// Make a point with a unit homogeneous coordinate.
    inline EPoint euclidean(const Point& p) {
        float a = 1 / p.w;
        return {a * p.x, a * p.y};
    }

    /// Make a line with a unit normal.
    inline NLine normalize(const Line& l) {
        float a = 1 / std::sqrt(l.x * l.x + l.y * l.y);
        return {a * l.x, a * l.y, a * l.w};
    }

    /// Calculate cross product.
    inline constexpr Vector cross(const Vector& u, const Vector& v) {
        return {u.y * v.w - u.w * v.y, u.w * v.x - u.x * v.w, u.x * v.y - u.y * v.x};
    }

    /// Calculate cross product.
    inline constexpr Vector cross(const EPoint& u, const Vector& v) {
        return {u.y * v.w - v.y, v.x - u.x * v.w, u.x * v.y - u.y * v.x};
    }

    /// Calculate cross product.
    inline constexpr Vector cross(const Vector& u, const EPoint& v) {
        return {u.y - u.w * v.y, u.w * v.x - u.x, u.x * v.y - u.y * v.x};
    }

    /// Calculate cross product.
    inline constexpr Vector cross(const EPoint& u, const EPoint& v) {
        return {u.y - v.y, v.x - u.x, u.x * v.y - u.y * v.x};
    }

    /// Calculate dot product.
    inline constexpr float dot(const Vector& u, const Vector& v) {
        return u.x * v.x + u.y * v.y + u.w * v.w;
    }

    /// Calculate dot product.
    inline constexpr float dot(const EPoint& u, const Vector& v) {
        return u.x * v.x + u.y * v.y + v.w;
    }

    /// Calculate dot product.
    inline constexpr float dot(const Vector& u, const EPoint& v) {
        return u.x * v.x + u.y * v.y + u.w;
    }

    /// Calculate dot product.
    inline constexpr float dot(const EPoint& u, const EPoint& v) {
        return u.x * v.x + u.y * v.y + 1;
    }

    /// Form a line by connecting two points.
    inline constexpr Line connect(const Point& p, const Point& q) { return Line{cross(p, q)}; }

    /// Form a line by connecting two points.
    inline constexpr Line connect(const EPoint& p, const Point& q) { return Line{cross(p, q)}; }

    /// Form a line by connecting two points.
    inline constexpr Line connect(const Point& p, const EPoint& q) { return Line{cross(p, q)}; }

    /// Form a line by connecting two points.
    inline constexpr Line connect(const EPoint& p, const EPoint& q) { return Line{cross(p, q)}; }

    /// Form a point by intersecting two lines.
    inline Point intersect(const Line& l1, const Line& l2) { return Point{cross(l1, l2)}; }

    /// Point-line squared distance.
    inline float sqr_distance(const EPoint& p, const NLine& l) {
        float d = p.x * l.x + p.y * l.y + l.w;
        return d * d;
    }

    /// Calculates absolute value for a scalar.
    template <typename T>
    inline constexpr T abs(T x) { return (x < 0) ? -x : x; }

    /// Checks that two scalars are equal.
    inline constexpr float same_scalar(float l, float r, float thresh = 1e-6f) {
        return abs(l - r) < thresh;
    }

    /// Checks that two vectors are equal.
    inline constexpr bool same_vector(const Vector& l, const Vector& r, float thresh = 1e-6f) {
        return abs(l.x - r.x) < thresh && abs(l.y - r.y) < thresh && abs(l.w - r.w) < thresh;
    }
}

#endif // FMO_ALGEBRA_HPP
