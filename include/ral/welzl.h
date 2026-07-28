#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <iostream>
#include <iomanip>
#include "random_utils.h"
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Welzl's Algorithm for Minimum Enclosing Circle (1991)
// Finds the smallest enclosing circle of a set of points in expected O(n) time.
// Uses randomized incremental construction.
// ---------------------------------------------------------------------------

struct Point2D_mec {
    double x, y;
    Point2D_mec(double x = 0, double y = 0) : x(x), y(y) {}

    bool operator==(const Point2D_mec& o) const {
        return std::abs(x - o.x) < 1e-12 && std::abs(y - o.y) < 1e-12;
    }
};

struct Circle {
    Point2D_mec center;
    double radius;
    Circle(Point2D_mec c = {0, 0}, double r = 0) : center(c), radius(r) {}

    bool contains(const Point2D_mec& p) const {
        double dx = p.x - center.x;
        double dy = p.y - center.y;
        return dx * dx + dy * dy <= radius * radius + 1e-9;
    }
};

inline double dist2_mec(const Point2D_mec& a, const Point2D_mec& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return dx * dx + dy * dy;
}

inline double dist_mec(const Point2D_mec& a, const Point2D_mec& b) {
    return std::sqrt(dist2_mec(a, b));
}

// Circle through 2 points: the circle with diameter p1-p2
inline Circle circle_from_2(const Point2D_mec& p1, const Point2D_mec& p2) {
    Point2D_mec c((p1.x + p2.x) / 2.0, (p1.y + p2.y) / 2.0);
    double r = dist_mec(p1, p2) / 2.0;
    return {c, r};
}

// Circle through 3 points (circumscribed circle)
inline Circle circle_from_3(const Point2D_mec& p1, const Point2D_mec& p2, const Point2D_mec& p3) {
    double ax = p1.x, ay = p1.y;
    double bx = p2.x, by = p2.y;
    double cx = p3.x, cy = p3.y;

    double D = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
    if (std::abs(D) < 1e-12) {
        // Points are nearly collinear
        double d12 = dist2_mec(p1, p2);
        double d13 = dist2_mec(p1, p3);
        double d23 = dist2_mec(p2, p3);
        if (d12 >= d13 && d12 >= d23) return circle_from_2(p1, p2);
        if (d13 >= d12 && d13 >= d23) return circle_from_2(p1, p3);
        return circle_from_2(p2, p3);
    }

    double ux = ((ax * ax + ay * ay) * (by - cy) +
                 (bx * bx + by * by) * (cy - ay) +
                 (cx * cx + cy * cy) * (ay - by)) / D;
    double uy = ((ax * ax + ay * ay) * (cx - bx) +
                 (bx * bx + by * by) * (ax - cx) +
                 (cx * cx + cy * cy) * (bx - ax)) / D;

    Point2D_mec center(ux, uy);
    double r = dist_mec(center, p1);
    return {center, r};
}

// Minimum enclosing circle using Welzl's algorithm (randomized incremental)
// O(n) expected time with random permutation
inline Circle welzl_mec(
    std::vector<Point2D_mec> points, std::mt19937& rng) {

    int n = static_cast<int>(points.size());
    if (n == 0) return {{0, 0}, 0};
    if (n == 1) return {points[0], 0};

    // Shuffle for expected linear time
    std::shuffle(points.begin(), points.end(), rng);

    // Start with circle through first two points
    Circle c = circle_from_2(points[0], points[1]);

    for (int i = 2; i < n; ++i) {
        if (!c.contains(points[i])) {
            // Point i must be on the boundary of the new MEC
            // Recompute with point i forced on boundary
            c = circle_from_2(points[0], points[i]);
            for (int j = 1; j < i; ++j) {
                if (!c.contains(points[j])) {
                    // Points i and j must be on the boundary
                    c = circle_from_2(points[i], points[j]);
                    for (int k = 0; k < j; ++k) {
                        if (!c.contains(points[k])) {
                            // All three i, j, k on the boundary
                            c = circle_from_3(points[i], points[j], points[k]);
                        }
                    }
                }
            }
        }
    }
    return c;
}

// Welzl's algorithm with boundary set tracking (recursive version)
inline Circle welzl_mec_recursive(
    std::vector<Point2D_mec>& points,
    std::vector<Point2D_mec> boundary,
    int n) {

    if (n == 0 || boundary.size() == 3) {
        if (boundary.empty()) return {{0, 0}, 0};
        if (boundary.size() == 1) return {boundary[0], 0};
        if (boundary.size() == 2) return circle_from_2(boundary[0], boundary[1]);
        return circle_from_3(boundary[0], boundary[1], boundary[2]);
    }

    // Pick a random point
    int idx = n - 1;

    Circle c = welzl_mec_recursive(points, boundary, n - 1);

    if (!c.contains(points[idx])) {
        boundary.push_back(points[idx]);
        c = welzl_mec_recursive(points, boundary, n - 1);
    }
    return c;
}

inline Circle welzl_mec_recursive_wrapper(
    std::vector<Point2D_mec> points, std::mt19937& rng) {

    std::shuffle(points.begin(), points.end(), rng);
    std::vector<Point2D_mec> boundary;
    return welzl_mec_recursive(points, boundary, static_cast<int>(points.size()));
}

// Demonstration
inline void demonstrate_welzl_mec() {
    std::mt19937 rng(42);

    println("=== Welzl's Minimum Enclosing Circle ===\n");

    // Test 1: Simple triangle
    println("--- Test 1: Equilateral Triangle ---");
    {
        std::vector<Point2D_mec> pts = {{0, 0}, {4, 0}, {2, 3.46}};
        auto c = welzl_mec(pts, rng);
        println("  Points: (0,0), (4,0), (2,3.46)");
        println("  Center: ({:.4f}, {:.4f})", c.center.x, c.center.y);
        println("  Radius: {:.4f}", c.radius);
        println("  Expected radius (circumradius): {:.4f}", 4.0 / std::sqrt(3.0));
        bool all_inside = true;
        for (auto& p : pts) all_inside &= c.contains(p);
        println("  All points inside: {}", all_inside ? "YES" : "NO");
    }

    // Test 2: Square
    println("\n--- Test 2: Unit Square ---");
    {
        std::vector<Point2D_mec> pts = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        auto c = welzl_mec(pts, rng);
        println("  Points: (0,0), (1,0), (1,1), (0,1)");
        println("  Center: ({:.4f}, {:.4f})", c.center.x, c.center.y);
        println("  Radius: {:.4f}", c.radius);
        println("  Expected radius: {:.4f}", std::sqrt(2.0) / 2.0);
    }

    // Test 3: Random points
    println("\n--- Test 3: 20 Random Points ---");
    {
        std::uniform_real_distribution<double> dist(-10.0, 10.0);
        std::vector<Point2D_mec> pts(20);
        for (auto& p : pts) p = {dist(rng), dist(rng)};

        auto c = welzl_mec(pts, rng);
        println("  Center: ({:.4f}, {:.4f})", c.center.x, c.center.y);
        println("  Radius: {:.4f}", c.radius);

        bool all_inside = true;
        for (auto& p : pts) all_inside &= c.contains(p);
        println("  All points inside: {}", all_inside ? "YES" : "NO");

        // Verify with naive O(n^4) approach
        double min_r = 1e18;
        for (int i = 0; i < (int)pts.size(); ++i) {
            for (int j = i + 1; j < (int)pts.size(); ++j) {
                for (int k = j + 1; k < (int)pts.size(); ++k) {
                    auto c3 = circle_from_3(pts[i], pts[j], pts[k]);
                    bool covers = true;
                    for (auto& p : pts) covers &= c3.contains(p);
                    if (covers && c3.radius < min_r) min_r = c3.radius;
                }
            }
        }
        for (int i = 0; i < (int)pts.size(); ++i) {
            for (int j = i + 1; j < (int)pts.size(); ++j) {
                auto c2 = circle_from_2(pts[i], pts[j]);
                bool covers = true;
                for (auto& p : pts) covers &= c2.contains(p);
                if (covers && c2.radius < min_r) min_r = c2.radius;
            }
        }
        println("  Radius matches brute-force: {}",
                      std::abs(c.radius - min_r) < 1e-6 ? "YES" : "NO");
    }

    // Test 4: Performance
    println("\n--- Test 4: Performance ---");
    {
        for (int n : {100, 1000, 10000}) {
            std::uniform_real_distribution<double> dist(-1000.0, 1000.0);
            std::vector<Point2D_mec> pts(n);
            for (auto& p : pts) p = {dist(rng), dist(rng)};

            auto t1 = std::chrono::high_resolution_clock::now();
            auto c = welzl_mec(pts, rng);
            auto t2 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

            println("  n={}: radius={:.2f}, time={:.3f} ms", n, c.radius, ms);
        }
    }
}

} // namespace ral
