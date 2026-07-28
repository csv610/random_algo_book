// =============================================================================
// Geometric Primitives: Point Location, Orientation Tests, and Applications
// =============================================================================
//
// Demonstrates fundamental geometric predicates used throughout computational
// geometry:
//
// 1. Orientation test (cross product): determines if three points make a
//    left turn, right turn, or are collinear.
//
// 2. Point-in-polygon test (ray casting): determines if a point is inside,
//    outside, or on the boundary of a polygon.
//
// 3. Segment intersection: tests if two line segments cross.
//
// 4. In-circle test: determines if a point lies inside the circumcircle
//    of a triangle (used in Delaunay triangulation).
//
// These primitives are the building blocks for convex hulls, triangulations,
// and other geometric algorithms.
// =============================================================================

#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <cmath>
#include <algorithm>

#include "ral/convex_hull.h"
#include "ral/delaunay.h"
#include "ral/binary_planar_partition.h"

using namespace ral;

// --- Point-in-polygon via ray casting ---
// Cast a ray from the point to the right (+x direction) and count crossings.
// Odd number of crossings = inside, even = outside.
inline bool point_in_polygon(const Point2D& p, const std::vector<Point2D>& polygon) {
    int n = static_cast<int>(polygon.size());
    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        const auto& pi = polygon[i];
        const auto& pj = polygon[j];

        // Check if the edge crosses the horizontal ray from p
        if (((pi.y > p.y) != (pj.y > p.y)) &&
            (p.x < (pj.x - pi.x) * (p.y - pi.y) / (pj.y - pi.y) + pi.x)) {
            inside = !inside;
        }
    }
    return inside;
}

// --- Orientation test ---
// Returns > 0 for left turn (CCW), < 0 for right turn (CW), 0 for collinear.
inline double orientation_test(const Point2D& a, const Point2D& b, const Point2D& c) {
    return cross_product(a, b, c);
}

// --- Point on segment test ---
inline bool on_segment(const Point2D& p, const Point2D& a, const Point2D& b) {
    double cross = cross_product(a, b, p);
    if (std::abs(cross) > 1e-9) return false;
    return p.x >= std::min(a.x, b.x) - 1e-9 &&
           p.x <= std::max(a.x, b.x) + 1e-9 &&
           p.y >= std::min(a.y, b.y) - 1e-9 &&
           p.y <= std::max(a.y, b.y) + 1e-9;
}

int main() {
    std::cout << "=== Geometric Primitives: Point Location & Orientation ===\n\n";

    // --- Demo 1: Orientation test ---
    std::cout << "Demo 1: Orientation (cross product) test\n";
    {
        Point2D a(0, 0), b(1, 0);

        Point2D c1(0, 1);   // left of a->b
        Point2D c2(0, -1);  // right of a->b
        Point2D c3(2, 0);   // collinear

        std::cout << "  Points: a=(0,0), b=(1,0)\n";
        std::cout << "  a,b,(0,1): " << orientation_test(a, b, c1)
                  << " (positive = left turn = CCW)\n";
        std::cout << "  a,b,(0,-1): " << orientation_test(a, b, c2)
                  << " (negative = right turn = CW)\n";
        std::cout << "  a,b,(2,0): " << orientation_test(a, b, c3)
                  << " (zero = collinear)\n\n";
    }

    // --- Demo 2: Point in convex polygon ---
    std::cout << "Demo 2: Point in convex polygon (all-left-turn test)\n";
    {
        // Unit square
        std::vector<Point2D> square = {{0,0}, {1,0}, {1,1}, {0,1}};

        std::vector<Point2D> test_points = {
            {0.5, 0.5},  // inside
            {2.0, 2.0},  // outside
            {0.5, 0.0},  // on edge
            {-0.1, 0.5}  // outside
        };

        for (auto& p : test_points) {
            bool inside = point_in_polygon(p, square);
            std::cout << "  (" << p.x << ", " << p.y << ") -> "
                      << (inside ? "INSIDE" : "OUTSIDE") << "\n";
        }
        std::cout << "\n";
    }

    // --- Demo 3: Point in star-shaped polygon ---
    std::cout << "Demo 3: Point in non-convex (star-shaped) polygon\n";
    {
        // 5-pointed star
        std::vector<Point2D> star;
        for (int i = 0; i < 10; i++) {
            double angle = M_PI / 2 + i * M_PI / 5.0;
            double r = (i % 2 == 0) ? 1.0 : 0.4;
            star.push_back({r * std::cos(angle), r * std::sin(angle)});
        }

        std::vector<Point2D> test = {
            {0.0, 0.0},   // center (inside star)
            {0.0, 0.9},   // outer tip (outside)
            {0.3, 0.0},   // between inner and outer (inside)
            {0.0, 0.5},   // on the arm (inside)
        };

        std::cout << "  5-pointed star polygon (10 vertices)\n";
        for (auto& p : test) {
            bool inside = point_in_polygon(p, star);
            std::cout << "  (" << p.x << ", " << p.y << ") -> "
                      << (inside ? "INSIDE" : "OUTSIDE") << "\n";
        }
        std::cout << "\n";
    }

    // --- Demo 4: On-segment test ---
    std::cout << "Demo 4: On-segment test\n";
    {
        Point2D a(0, 0), b(10, 0);
        Point2D p1(5, 0), p2(5, 5), p3(0, 0), p4(-1, 0);

        std::cout << "  Segment: (0,0) -> (10,0)\n";
        std::cout << "  (5,0):   " << (on_segment(p1, a, b) ? "ON" : "NOT ON") << " segment\n";
        std::cout << "  (5,5):   " << (on_segment(p2, a, b) ? "ON" : "NOT ON") << " segment\n";
        std::cout << "  (0,0):   " << (on_segment(p3, a, b) ? "ON" : "NOT ON") << " segment (endpoint)\n";
        std::cout << "  (-1,0):  " << (on_segment(p4, a, b) ? "ON" : "NOT ON") << " segment\n\n";
    }

    // --- Demo 5: Segment intersection (from binary_planar_partition.h) ---
    std::cout << "Demo 5: Segment intersection test\n";
    {
        Segment s1(0, 0, 4, 4, 0);  // diagonal
        Segment s2(0, 4, 4, 0, 1);  // crossing diagonal
        Segment s3(5, 0, 5, 5, 2);  // vertical (not intersecting)

        std::cout << "  s1: (0,0)->(4,4),  s2: (0,4)->(4,0),  s3: (5,0)->(5,5)\n";
        std::cout << "  s1 intersects s2: " << (segments_intersect(s1, s2) ? "YES" : "NO") << "\n";
        std::cout << "  s1 intersects s3: " << (segments_intersect(s1, s3) ? "YES" : "NO") << "\n";
        std::cout << "  s2 intersects s3: " << (segments_intersect(s2, s3) ? "YES" : "NO") << "\n\n";
    }

    // --- Demo 6: In-circle test (from delaunay.h) ---
    std::cout << "Demo 6: In-circle test\n";
    {
        Point2D a(0, 0), b(4, 0), c(0, 4);
        Point2D inside(1, 1);
        Point2D outside(5, 5);

        double ic_in  = in_circumcircle(inside, a, b, c);
        double ic_out = in_circumcircle(outside, a, b, c);

        std::cout << "  Triangle: (0,0), (4,0), (0,4)\n";
        std::cout << "  Circumcircle center: (2, 2), radius: ~2.83\n";
        std::cout << "  Point (1,1) in circumcircle: " << (ic_in > 0 ? "YES" : "NO") << "\n";
        std::cout << "  Point (5,5) in circumcircle: " << (ic_out > 0 ? "YES" : "NO") << "\n\n";
    }

    // --- Demo 7: Practical application — polygon area via cross product ---
    std::cout << "Demo 7: Polygon area via cross product\n";
    {
        std::vector<Point2D> poly = {{0,0}, {4,0}, {4,3}, {0,3}}; // 4x3 rectangle
        double area = 0;
        int n = static_cast<int>(poly.size());
        for (int i = 0; i < n; i++) {
            int j = (i + 1) % n;
            area += poly[i].x * poly[j].y;
            area -= poly[j].x * poly[i].y;
        }
        area = std::abs(area) / 2.0;

        std::cout << "  Rectangle (0,0)-(4,0)-(4,3)-(0,3)\n";
        std::cout << "  Signed area: " << area << " (expected 12.0)\n\n";
    }

    std::cout << "Done.\n";
    return 0;
}
