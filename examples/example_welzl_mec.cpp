// =============================================================================
// Welzl's Minimum Enclosing Circle — Iterative (Randomized Incremental)
// =============================================================================
//
// Finds the smallest circle that encloses all given 2D points.
// Uses the iterative variant of Welzl's algorithm:
//  1. Shuffle points randomly (for expected O(n) time).
//  2. Start with the circle through the first two points.
//  3. For each subsequent point, if it's outside the current circle,
//     rebuild the circle with that point on the boundary, checking
//     all previous points.
//
// The circle is defined by at most 3 boundary points:
//  - 0 boundary points: empty (radius 0)
//  - 1 boundary point:  degenerate (radius 0)
//  - 2 boundary points: diameter = the two points
//  - 3 boundary points: circumscribed circle
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

    std::cout << "=== Welzl's Minimum Enclosing Circle (Iterative) ===\n\n";

    // --- Demo 1: Equilateral triangle ---
    std::cout << "Demo 1: Equilateral triangle with side 4\n";
    {
        std::vector<Point2D_mec> pts = {{0, 0}, {4, 0}, {2, 3.46}};
        auto c = welzl_mec(pts, rng);

        double expected_r = 4.0 / std::sqrt(3.0);
        std::cout << "  Points: (0,0), (4,0), (2,3.46)\n";
        std::cout << "  Center: (" << std::fixed << std::setprecision(4)
                  << c.center.x << ", " << c.center.y << ")\n";
        std::cout << "  Radius: " << c.radius << "\n";
        std::cout << "  Expected circumradius: " << expected_r << "\n";
        std::cout << "  Match: " << (std::abs(c.radius - expected_r) < 0.01 ? "YES" : "NO") << "\n\n";
    }

    // --- Demo 2: Unit square ---
    std::cout << "Demo 2: Unit square\n";
    {
        std::vector<Point2D_mec> pts = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        auto c = welzl_mec(pts, rng);

        double expected_r = std::sqrt(2.0) / 2.0;
        std::cout << "  Center: (" << c.center.x << ", " << c.center.y << ")\n";
        std::cout << "  Radius: " << c.radius << "\n";
        std::cout << "  Expected (half-diagonal): " << expected_r << "\n\n";
    }

    // --- Demo 3: Random points — verify all inside ---
    std::cout << "Demo 3: 100 random points — verify containment\n";
    {
        std::uniform_real_distribution<double> dist(-50.0, 50.0);
        std::vector<Point2D_mec> pts(100);
        for (auto& p : pts) p = {dist(rng), dist(rng)};

        auto c = welzl_mec(pts, rng);

        bool all_inside = true;
        for (auto& p : pts) {
            if (!c.contains(p)) { all_inside = false; break; }
        }

        std::cout << "  All points inside circle: " << (all_inside ? "YES" : "NO") << "\n";
        std::cout << "  Center: (" << c.center.x << ", " << c.center.y << ")\n";
        std::cout << "  Radius: " << c.radius << "\n\n";
    }

    // --- Demo 4: Brute-force verification ---
    std::cout << "Demo 4: Verify against O(n^4) brute-force (20 points)\n";
    {
        std::uniform_real_distribution<double> dist(-10.0, 10.0);
        std::vector<Point2D_mec> pts(20);
        for (auto& p : pts) p = {dist(rng), dist(rng)};

        auto c_welzl = welzl_mec(pts, rng);

        // Brute force: try all pairs and triples
        double min_r = 1e18;
        for (size_t i = 0; i < pts.size(); i++) {
            for (size_t j = i+1; j < pts.size(); j++) {
                auto c2 = circle_from_2(pts[i], pts[j]);
                bool covers = true;
                for (auto& p : pts) covers &= c2.contains(p);
                if (covers && c2.radius < min_r) min_r = c2.radius;

                for (size_t k = j+1; k < pts.size(); k++) {
                    auto c3 = circle_from_3(pts[i], pts[j], pts[k]);
                    covers = true;
                    for (auto& p : pts) covers &= c3.contains(p);
                    if (covers && c3.radius < min_r) min_r = c3.radius;
                }
            }
        }

        std::cout << "  Welzl radius:    " << c_welzl.radius << "\n";
        std::cout << "  Brute-force min: " << min_r << "\n";
        std::cout << "  Match: " << (std::abs(c_welzl.radius - min_r) < 1e-6 ? "YES" : "NO") << "\n\n";
    }

    // --- Demo 5: Performance ---
    std::cout << "Demo 5: Performance scaling\n";
    std::cout << std::setw(10) << "  n"
              << std::setw(14) << "radius"
              << std::setw(14) << "time (ms)" << "\n";
    std::cout << "  " << std::string(38, '-') << "\n";

    for (int n : {100, 1000, 10000, 100000}) {
        std::uniform_real_distribution<double> dist(-1000.0, 1000.0);
        std::vector<Point2D_mec> pts(n);
        for (auto& p : pts) p = {dist(rng), dist(rng)};

        auto t0 = std::chrono::high_resolution_clock::now();
        auto c = welzl_mec(pts, rng);
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        std::cout << std::setw(10) << n
                  << std::setw(14) << std::fixed << std::setprecision(2) << c.radius
                  << std::setw(14) << std::setprecision(3) << ms << "\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}
