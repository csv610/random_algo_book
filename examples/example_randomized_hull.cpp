// =============================================================================
// Randomized Incremental Convex Hull
// =============================================================================
//
// Computes the convex hull by sorting points by x-coordinate and building
// upper and lower chains incrementally. Each new point is appended and
// non-hull interior points are removed via cross-product tests.
//
// Expected time complexity: O(n log n) due to the initial sort.
// Works well for arbitrary point sets regardless of hull size.
//
// The algorithm maintains two monotone chains (upper and lower) and merges
// them at the end to produce the full convex hull in counterclockwise order.
// =============================================================================

#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <cmath>
#include <chrono>

#include "ral/convex_hull.h"

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Randomized Incremental Convex Hull ===\n\n";

    // --- Demo 1: Simple known input ---
    std::cout << "Demo 1: Points on a unit square + interior points\n";
    {
        std::vector<Point2D> points = {
            {0, 0}, {1, 0}, {1, 1}, {0, 1},   // corners
            {0.2, 0.3}, {0.7, 0.8}, {0.5, 0.5} // interior
        };

        auto hull = convex_hull(points);

        std::cout << "  Hull vertices: ";
        for (auto& p : hull)
            std::cout << "(" << p.x << "," << p.y << ") ";
        std::cout << "\n  Expected hull size: 4 (the four corners)\n";
        std::cout << "  Actual hull size:   " << hull.size() << "\n\n";
    }

    // --- Demo 2: Points on a circle (worst case for hull size) ---
    std::cout << "Demo 2: Points on a circle (all points are hull vertices)\n";
    {
        int n = 20;
        std::vector<Point2D> points;
        for (int i = 0; i < n; i++) {
            double angle = 2.0 * M_PI * i / n;
            points.push_back({std::cos(angle), std::sin(angle)});
        }

        auto hull = convex_hull(points);

        std::cout << "  Input: " << n << " points on unit circle\n";
        std::cout << "  Hull size: " << hull.size() << " (expected " << n << ")\n\n";
    }

    // --- Demo 3: Scaling performance ---
    std::cout << "Demo 3: Scaling performance\n";
    std::cout << std::setw(10) << "  n"
              << std::setw(14) << "hull_size"
              << std::setw(14) << "time (ms)" << "\n";
    std::cout << "  " << std::string(38, '-') << "\n";

    for (int n : {100, 1000, 10000, 100000}) {
        std::uniform_real_distribution<double> dist(-1000.0, 1000.0);
        std::vector<Point2D> points(n);
        for (auto& p : points) p = {dist(rng), dist(rng)};

        auto t0 = std::chrono::high_resolution_clock::now();
        auto hull = convex_hull(points);
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        std::cout << std::setw(10) << n
                  << std::setw(14) << hull.size()
                  << std::setw(14) << std::fixed << std::setprecision(3) << ms << "\n";
    }

    // --- Demo 4: Many collinear points ---
    std::cout << "\nDemo 4: Collinear points (degenerate case)\n";
    {
        std::vector<Point2D> points;
        for (int i = 0; i < 10; i++)
            points.push_back({static_cast<double>(i), 0.0});
        points.push_back({5.0, 0.001}); // slight deviation

        auto hull = convex_hull(points);
        std::cout << "  Input: 11 points (10 collinear + 1 slight deviation)\n";
        std::cout << "  Hull size: " << hull.size() << "\n";
        std::cout << "  Hull vertices: ";
        for (auto& p : hull)
            std::cout << "(" << p.x << "," << p.y << ") ";
        std::cout << "\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}
