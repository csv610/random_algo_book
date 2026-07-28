// =============================================================================
// Jarvis March (Gift Wrapping) Convex Hull Algorithm
// =============================================================================
//
// Finds the convex hull of a set of 2D points by "wrapping" around the
// outermost points. Starting from the leftmost point, it repeatedly finds
// the next point that makes the smallest left turn (or is farthest in the
// case of collinear points), until it wraps back to the start.
//
// Time complexity: O(nh), where h is the number of hull vertices.
// Best when h is small relative to n.
//
// This contrasts with the randomized incremental hull which achieves
// expected O(n log n) regardless of hull size.
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

    std::cout << "=== Jarvis March (Gift Wrapping) Convex Hull ===\n\n";

    // --- Demo 1: Known small input ---
    std::cout << "Demo 1: Hand-crafted points\n";
    {
        std::vector<Point2D> points = {
            {0, 0}, {4, 0}, {4, 3}, {0, 3},   // rectangle corners
            {2, 1}, {2, 2},                      // interior points
            {1, -1}, {5, 4}                      // outliers
        };

        auto hull = gift_wrap_hull(points);

        std::cout << "  Input points: ";
        for (auto& p : points)
            std::cout << "(" << p.x << "," << p.y << ") ";
        std::cout << "\n  Hull vertices (CCW): ";
        for (auto& p : hull)
            std::cout << "(" << p.x << "," << p.y << ") ";
        std::cout << "\n  Hull size: " << hull.size() << "\n\n";
    }

    // --- Demo 2: Compare gift wrap vs randomized hull ---
    std::cout << "Demo 2: Timing comparison — gift_wrap_hull vs convex_hull\n";
    std::cout << std::left << std::setw(12) << "  n"
              << std::setw(14) << "gift_wrap"
              << std::setw(14) << "rand_incr"
              << std::setw(12) << "hull_size" << "\n";
    std::cout << "  " << std::string(50, '-') << "\n";

    for (int n : {50, 200, 1000, 5000}) {
        std::uniform_real_distribution<double> dist(-1000.0, 1000.0);
        std::vector<Point2D> points(n);
        for (auto& p : points) p = {dist(rng), dist(rng)};

        auto t0 = std::chrono::high_resolution_clock::now();
        auto hull1 = gift_wrap_hull(points);
        auto t1 = std::chrono::high_resolution_clock::now();
        auto hull2 = convex_hull(points);
        auto t2 = std::chrono::high_resolution_clock::now();

        double ms_wrap  = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double ms_incr  = std::chrono::duration<double, std::milli>(t2 - t1).count();

        std::cout << "  " << std::left
                  << std::setw(12) << n
                  << std::setw(14) << std::fixed << std::setprecision(3) << ms_wrap
                  << std::setw(14) << ms_incr
                  << std::setw(12) << hull1.size() << "\n";
    }

    // --- Demo 3: Verify hull property ---
    std::cout << "\nDemo 3: Hull verification (all turns should be CCW)\n";
    {
        std::uniform_real_distribution<double> dist(-100.0, 100.0);
        std::vector<Point2D> points(30);
        for (auto& p : points) p = {dist(rng), dist(rng)};

        auto hull = gift_wrap_hull(points);
        int h = static_cast<int>(hull.size());

        bool valid = true;
        for (int i = 0; i < h; i++) {
            double cp = cross_product(hull[i], hull[(i+1)%h], hull[(i+2)%h]);
            if (cp < -1e-9) {
                std::cout << "  WARNING: clockwise turn at vertex " << i << "\n";
                valid = false;
            }
        }
        if (valid) std::cout << "  All turns are CCW. Hull is valid.\n";

        // Check that all input points lie inside or on the hull
        bool all_inside = true;
        for (auto& p : points) {
            bool inside = false;
            for (int i = 0; i < h; i++) {
                double cp = cross_product(hull[i], hull[(i+1)%h], p);
                if (cp >= -1e-9) { inside = true; break; }
            }
            // Points on the boundary are OK too
            if (!inside) {
                for (auto& hp : hull) {
                    if (std::abs(p.x - hp.x) < 1e-9 && std::abs(p.y - hp.y) < 1e-9) {
                        inside = true;
                        break;
                    }
                }
            }
            if (!inside) { all_inside = false; break; }
        }
        std::cout << "  All input points are inside/on hull: "
                  << (all_inside ? "YES" : "NO") << "\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}
