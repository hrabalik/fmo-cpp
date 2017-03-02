#ifndef FMO_ALGEBRA_HPP
#define FMO_ALGEBRA_HPP

namespace fmo {
    struct Point;
    struct Vector;
    struct Line;

    /// Vector in projective coordinates.
    struct Vector {
        constexpr Vector(float aX, float aY, float aW) : x(aX), y(aY), w(aW) {}

        // data
        float x, y, w;
    };

    inline constexpr Vector cross(Vector u, Vector v) {
        return {u.y * v.w - u.w * v.y, u.w * v.x - u.x * v.w, u.x * v.y - u.y * v.x};
    }

    /// Point in Euclidean coordinates.
    struct Point {
        constexpr Point(float aX, float aY) : x(aX), y(aY) {}
        constexpr explicit Point(Vector v) : x(v.x / v.w), y(v.y / v.w) {}
        constexpr explicit operator Vector() const { return {x, y, 1}; }

        Point(const Point&) = default;
        Point& operator=(const Point&) = default;

        // data
        float x, y;
    };

    /// A line, represented as its normal in projective coordinates.
    struct Line {
        constexpr Line(float aX, float aY, float aW) : normal(aX, aY, aW) {}
        constexpr explicit Line(Vector v) : normal(v) {}
        constexpr explicit operator Vector() const { return normal; }

        Line(const Line&) = default;
        Line& operator=(const Line&) = default;

        // data
        Vector normal;
    };

    /// Form a line by connectiong two points.
    inline constexpr Line connect(Point p, Point q) {
        return {p.y - q.y, q.x - p.x, p.x * q.y - p.y * q.x};
    }

    /// Form a point by intersecting two lines.
    inline constexpr Point intersect(Line l1, Line l2) {
        return Point(cross(l1.normal, l2.normal));
    }
}

#endif // FMO_ALGEBRA_HPP
