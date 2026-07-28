// =============================================================================
// Welzl's Minimum Enclosing Circle — Recursive Variant
// =============================================================================
//
// A recursive implementation of Welzl's algorithm for the minimum enclosing
// circle. Unlike the iterative version, this one explicitly tracks the
// "boundary set" R of points that must lie on the circle boundary.
//
// Base cases:
//  - |R| = 3: return circumscribed circle of the 3 boundary points
//  - |R| = 2: return circle with diameter = the 2 points
//  - |R| = 1: return degenerate circle (radius 0)
//  - n = 0:   return degenerate circle
//
// Recursive step:
//  1. Pick the last point p in the random permutation.
//  2. Recursively solve for the first n-1 points.
//  3. If p is inside the resulting circle, we're done.
//  4. If p is outside, add p to the boundary set R and re-solve.
//
// Expected time complexity: O(n).
// =============================================================================

#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <cmath>
#include <chrono>

#include "ral/welzl.h"

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Welzl's Minimum Enclosing Circle (Recursive) ===\n\n";

    // --- Demo 1: Compare recursive vs iterative on same input ---
    std::cout << "Demo 1: Recursive vs Iterative on same 10 points\n";
    {
        std::uniform_real_distribution<double> dist(-20.0, 20.0);
        std::vector<Point2D_mec> pts(10);
        for (auto& p : pts) p = {dist(rng), dist(rng)};

        auto c_iter = welzl_mec(pts, rng);
        auto c_rec  = welzl_mec_recursive_wrapper(pts, rng);

        std::cout << "  Iterative: center=(" << c_iter.center.x << ", "
                  << c_iter.center.y << "), r=" << c_iter.radius << "\n";
        std::cout << "  Recursive: center=(" << c_rec.center.x << ", "
                  << c_rec.center.y << "), r=" << c_rec.radius << "\n";
        std::cout << "  Radii match: "
                  << (std::abs(c_iter.radius - c_rec.radius) < 1e-9 ? "YES" : "NO") << "\n\n";
    }

    // --- Demo 2: Points on a line (degenerate) ---
    std::cout << "Demo 2: Collinear points (degenerate case)\n";
    {
        std::vector<Point2D_mec> pts = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}};
        auto c = welzl_mec_recursive_wrapper(pts, rng);

        std::cout << "  5 collinear points from (0,0) to (4,0)\n";
        std::cout << "  Center: (" << c.center.x << ", " << c.center.y << ")\n";
        std::cout << "  Radius: " << c.radius << " (expected 2.0)\n";
        std::cout << "  Match: " << (std::abs(c.radius - 2.0) < 0.01 ? "YES" : "NO") << "\n\n";
    }

    // --- Demo 3: Random larger set with verification ---
    std::cout << "Demo 3: 50 random points — both algorithms agree\n";
    {
        std::uniform_real_distribution<double> dist(-100.0, 100.0);
        std::vector<Point2D_mec> pts(50);
        for (auto& p : pts) p = {dist(rng), dist(rng)};

        auto c_iter = welzl_mec(pts, rng);
        auto c_rec  = welzl_mec_recursive_wrapper(pts, rng);

        bool all_inside_iter = true, all_inside_rec = true;
        for (auto& p : pts) {
            all_inside_iter &= c_iter.contains(p);
            all_inside_rec  &= c_rec.contains(p);
        }

        std::cout << "  Iterative covers all: " << (all_inside_iter ? "YES" : "NO") << "\n";
        std::cout << "  Recursive covers all: " << (all_inside_rec ? "YES" : "NO") << "\n";
        std::cout << "  Radii agree: "
                  << (std::abs(c_iter.radius - c_rec.radius) < 1e-6 ? "YES" : "NO") << "\n\n";
    }

    // --- Demo 4: Pentagon (regular polygon) ---
    std::cout << "Demo 4: Regular pentagon (circumscribed circle should match)\n";
    {
        std::vector<Point2D_mec> pts;
        for (int i = 0; i < 5; i++) {
            double angle = 2.0 * M_PI * i / 5.0;
            pts.push_back({std::cos(angle), std::sin(angle)});
        }
        auto c = welzl_mec_recursive_wrapper(pts, rng);

        std::cout << "  Center: (" << c.center.x << ", " << c.center.y << ")\n";
        std::cout << "  Radius: " << std::fixed << std::setprecision(6) << c.radius << "\n";
        std::cout << "  Expected (unit circle): 1.000000\n";
        std::cout << "  Match: " << (std::abs(c.radius - 1.0) < 0.01 ? "YES" : "NO") << "\n\n";
    }

    // --- Demo 5: Performance comparison ---
    std::cout << "Demo 5: Performance — recursive vs iterative\n";
    std::cout << std::setw(10) << "  n"
              << std::setw(14) << "iter (ms)"
              << std::setw(14) << "rec (ms)" << "\n";
    std::cout << "  " << std::string(38, '-') << "\n";

    for (int n : {100, 500, 2000, 5000}) {
        std::uniform_real_distribution<double> dist(-500.0, 500.0);
        std::vector<Point2D_mec> pts(n);
        for (auto& p : pts) p = {dist(rng), dist(rng)};

        auto t0 = std::chrono::high_resolution_clock::now();
        welzl_mec(pts, rng);
        auto t1 = std::chrono::high_resolution_clock::now();
        welzl_mec_recursive_wrapper(pts, rng);
        auto t2 = std::chrono::high_resolution_clock::now();

        double ms_iter = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double ms_rec  = std::chrono::duration<double, std::milli>(t2 - t1).count();

        std::cout << std::setw(10) << n
                  << std::setw(14) << std::fixed << std::setprecision(3) << ms_iter
                  << std::setw(14) << ms_rec << "\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}
