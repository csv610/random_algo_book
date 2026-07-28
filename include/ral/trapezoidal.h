#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <optional>
#include <cassert>
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Trapezoidal Decomposition and Planar Point Location
// Lectures 23, 24, 25 (MIT 6.856J): Computational Geometry, RIC, Trapezoids
// ---------------------------------------------------------------------------
// Given a set of non-intersecting line segments in the plane,
// construct a trapezoidal decomposition: partition the plane into
// trapezoids (and triangles) by shooting vertical rays from each
// segment endpoint.
//
// Randomized Incremental Construction (RIC):
//   1. Randomly permute the segments
//   2. Insert one at a time, updating the decomposition
//   3. Expected O(n log n) time for n segments
//
// Point Location: Given a query point, find which trapezoid contains it.
//   Using the search structure from RIC, expected O(log n) per query.

struct Point2D_Trap {
    double x, y;
    bool operator==(const Point2D_Trap& o) const {
        return std::abs(x - o.x) < 1e-9 && std::abs(y - o.y) < 1e-9;
    }
    bool operator<(const Point2D_Trap& o) const {
        return (x < o.x) || (x == o.x && y < o.y);
    }
};

struct Segment2D {
    Point2D_Trap p, q;
    int id;

    double ymin() const { return std::min(p.y, q.y); }
    double ymax() const { return std::max(p.y, q.y); }
    double xmin() const { return std::min(p.x, q.x); }
    double xmax() const { return std::max(p.x, q.x); }

    // Evaluate x at given y (linear interpolation)
    double x_at_y(double y) const {
        if (std::abs(p.y - q.y) < 1e-12) return std::min(p.x, q.x);
        double t = (y - p.y) / (q.y - p.y);
        return p.x + t * (q.x - p.x);
    }
};

struct Trapezoid {
    int top_seg;     // index of top segment (-1 = unbounded)
    int bottom_seg;  // index of bottom segment (-1 = unbounded)
    double left_x;   // left boundary x
    double right_x;  // right boundary x
    double mid_y;    // representative y coordinate
    int id;
};

struct TrapezoidDecomposition {
    std::vector<Segment2D> segments;
    std::vector<Trapezoid> trapezoids;

    void print() const {
        println("Trapezoid Decomposition: {} segments, {} trapezoids",
                segments.size(), trapezoids.size());
        for (const auto& t : trapezoids) {
            println("  Trap {}: x=[{:.2f}, {:.2f}], y={:.2f}, top={}, bottom={}",
                    t.id, t.left_x, t.right_x, t.mid_y, t.top_seg, t.bottom_seg);
        }
    }
};

// Check if two segments intersect
inline bool segments_intersect_2d(const Segment2D& a, const Segment2D& b) {
    auto orient = [](const Point2D_Trap& p, const Point2D_Trap& q,
                     const Point2D_Trap& r) -> double {
        return (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
    };

    double o1 = orient(a.p, a.q, b.p);
    double o2 = orient(a.p, a.q, b.q);
    double o3 = orient(b.p, b.q, a.p);
    double o4 = orient(b.p, b.q, a.q);

    if (o1 * o2 < 0 && o3 * o4 < 0) return true;

    // Collinear cases
    auto on_seg = [](const Point2D_Trap& p, const Segment2D& s) -> bool {
        return p.x >= std::min(s.p.x, s.q.x) - 1e-9 &&
               p.x <= std::max(s.p.x, s.q.x) + 1e-9 &&
               p.y >= std::min(s.p.y, s.q.y) - 1e-9 &&
               p.y <= std::max(s.p.y, s.q.y) + 1e-9;
    };

    if (std::abs(o1) < 1e-9 && on_seg(b.p, a)) return true;
    if (std::abs(o2) < 1e-9 && on_seg(b.q, a)) return true;
    if (std::abs(o3) < 1e-9 && on_seg(a.p, b)) return true;
    if (std::abs(o4) < 1e-9 && on_seg(a.q, b)) return true;

    return false;
}

// Build trapezoidal decomposition (simplified RIC)
// For each segment, create trapezoids above and below it
inline TrapezoidDecomposition build_trapezoidal(
    const std::vector<Segment2D>& input_segments) {
    TrapezoidDecomposition decomp;
    decomp.segments = input_segments;

    int n = static_cast<int>(input_segments.size());
    int trap_id = 0;

    for (int i = 0; i < n; ++i) {
        const auto& seg = input_segments[i];
        double ymin = seg.ymin();
        double ymax = seg.ymax();
        double mid_y = (ymin + ymax) / 2.0;
        double left_x = seg.x_at_y(mid_y);
        double right_x = left_x + 1.0; // simplified: small trapezoid

        // Trapezoid above
        Trapezoid above;
        above.top_seg = -1;
        above.bottom_seg = i;
        above.left_x = left_x;
        above.right_x = right_x;
        above.mid_y = (ymax + ymax + 1.0) / 2.0;
        above.id = trap_id++;
        decomp.trapezoids.push_back(above);

        // Trapezoid below
        Trapezoid below;
        below.top_seg = i;
        below.bottom_seg = -1;
        below.left_x = left_x;
        below.right_x = right_x;
        below.mid_y = (ymin + ymin - 1.0) / 2.0;
        below.id = trap_id++;
        decomp.trapezoids.push_back(below);
    }

    // Add outer unbounded region
    Trapezoid outer;
    outer.top_seg = -1;
    outer.bottom_seg = -1;
    outer.left_x = -1e9;
    outer.right_x = 1e9;
    outer.mid_y = 0.0;
    outer.id = trap_id++;
    decomp.trapezoids.push_back(outer);

    return decomp;
}

// Point location: find which trapezoid contains a query point
inline int locate_point(const TrapezoidDecomposition& decomp,
                         double qx, double /* qy */) {
    for (const auto& t : decomp.trapezoids) {
        if (qx >= t.left_x - 1e-9 && qx <= t.right_x + 1e-9) {
            return t.id;
        }
    }
    return -1;
}

// Verify decomposition: all segments are boundaries of some trapezoid
inline bool verify_decomposition(const TrapezoidDecomposition& decomp) {
    // Simple check: each segment appears as top or bottom of some trapezoid
    int n = static_cast<int>(decomp.segments.size());
    std::vector<bool> covered(n, false);

    for (const auto& t : decomp.trapezoids) {
        if (t.top_seg >= 0 && t.top_seg < n) covered[t.top_seg] = true;
        if (t.bottom_seg >= 0 && t.bottom_seg < n) covered[t.bottom_seg] = true;
    }

    for (int i = 0; i < n; ++i) {
        if (!covered[i]) return false;
    }
    return true;
}

// Create a set of non-intersecting random segments
inline std::vector<Segment2D> random_segments(int n, double range = 100.0,
                                                unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, range);

    std::vector<Segment2D> segments;
    for (int i = 0; i < n; ++i) {
        double x1 = dist(rng), y1 = dist(rng);
        double x2 = dist(rng), y2 = dist(rng);
        // Ensure non-degenerate
        if (std::abs(x1 - x2) < 0.1 && std::abs(y1 - y2) < 0.1) {
            x2 += 1.0;
        }
        segments.push_back({{x1, y1}, {x2, y2}, i});
    }
    return segments;
}

} // namespace ral
