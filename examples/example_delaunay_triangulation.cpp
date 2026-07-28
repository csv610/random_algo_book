// =============================================================================
// Delaunay Triangulation (Randomized Incremental, Bowyer-Watson)
// =============================================================================
//
// Given a set of 2D points, produces a triangulation where no point lies
// inside the circumcircle of any triangle. This maximizes the minimum
// angle among all triangulations of the point set.
//
// Algorithm: Randomized incremental insertion with Bowyer-Watson cavity.
//  1. Start with a super-triangle enclosing all points.
//  2. Insert points in random order.
//  3. For each new point, find all triangles whose circumcircle contains
//     the point (the "bad" triangles), remove them to form a polygonal
//     cavity, and re-triangulate the cavity with the new point.
//  4. Remove triangles that share a vertex with the super-triangle.
//
// Expected time complexity: O(n log n).
// =============================================================================

#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <cmath>
#include <chrono>

#include "ral/delaunay.h"

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Delaunay Triangulation ===\n\n";

    // --- Demo 1: Small known input ---
    std::cout << "Demo 1: Points forming a square with a center point\n";
    {
        std::vector<Point2D> points = {
            {0, 0}, {4, 0}, {4, 4}, {0, 4}, {2, 2}
        };

        auto triangles = delaunay_triangulation(points);

        std::cout << "  Input: 5 points\n";
        std::cout << "  Triangles: " << triangles.size() << "\n";
        for (size_t i = 0; i < triangles.size(); i++) {
            const auto& t = triangles[i];
            std::cout << "    Triangle " << i << ": vertices ("
                      << t.v[0] << ", " << t.v[1] << ", " << t.v[2] << ")\n";
        }
        std::cout << "\n";
    }

    // --- Demo 2: Verify Delaunay property (no point in circumcircle) ---
    std::cout << "Demo 2: Verifying Delaunay property on random points\n";
    {
        int n = 50;
        std::uniform_real_distribution<double> dist(-100.0, 100.0);
        std::vector<Point2D> points(n);
        for (auto& p : points) p = {dist(rng), dist(rng)};

        auto triangles = delaunay_triangulation(points);

        int violations = 0;
        for (auto& tri : triangles) {
            for (int k = 0; k < n; k++) {
                if (tri.contains_vertex(k)) continue;
                double ic = in_circumcircle(points[k],
                    points[tri.v[0]], points[tri.v[1]], points[tri.v[2]]);
                if (ic > 1e-9) {
                    violations++;
                }
            }
        }

        std::cout << "  n=" << n << ", triangles=" << triangles.size() << "\n";
        std::cout << "  Delaunay violations: " << violations
                  << (violations == 0 ? " (all clean!)" : " (PROBLEM)") << "\n\n";
    }

    // --- Demo 3: Scaling performance ---
    std::cout << "Demo 3: Scaling performance\n";
    std::cout << std::setw(10) << "  n"
              << std::setw(14) << "triangles"
              << std::setw(14) << "time (ms)" << "\n";
    std::cout << "  " << std::string(38, '-') << "\n";

    for (int n : {100, 500, 2000, 10000}) {
        std::uniform_real_distribution<double> dist(-500.0, 500.0);
        std::vector<Point2D> points(n);
        for (auto& p : points) p = {dist(rng), dist(rng)};

        auto t0 = std::chrono::high_resolution_clock::now();
        auto triangles = delaunay_triangulation(points);
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        std::cout << std::setw(10) << n
                  << std::setw(14) << triangles.size()
                  << std::setw(14) << std::fixed << std::setprecision(3) << ms << "\n";
    }

    // --- Demo 4: Points on a grid (structured input) ---
    std::cout << "\nDemo 4: Grid points (5x5 grid)\n";
    {
        std::vector<Point2D> points;
        for (int i = 0; i < 5; i++)
            for (int j = 0; j < 5; j++)
                points.push_back({static_cast<double>(i), static_cast<double>(j)});

        auto triangles = delaunay_triangulation(points);
        std::cout << "  25 grid points -> " << triangles.size() << " triangles\n";
        std::cout << "  (For n=25 points with hull size 16, expected ~" << 2*25 - 16 - 2 << " triangles)\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}
