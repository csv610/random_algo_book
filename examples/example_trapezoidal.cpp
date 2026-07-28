#include "ral/trapezoidal.h"
#include <iostream>

int main() {
    using namespace ral;

    // Create some non-intersecting segments
    std::vector<Segment2D> segments;
    segments.push_back({{2.0, 1.0}, {4.0, 3.0}, 0});
    segments.push_back({{1.0, 2.0}, {3.0, 0.0}, 1});
    segments.push_back({{5.0, 0.5}, {7.0, 2.5}, 2});

    println("=== Trapezoidal Decomposition ===");
    auto decomp = build_trapezoidal(segments);
    decomp.print();

    println("Verification: {}", verify_decomposition(decomp) ? "PASS" : "FAIL");

    // Point location
    println("\n=== Point Location ===");
    for (auto [qx, qy] : std::vector<std::pair<double,double>>{{3.0, 2.0}, {6.0, 1.5}, {0.0, 0.0}}) {
        int trap = locate_point(decomp, qx, qy);
        println("  Point ({}, {}) -> trapezoid {}", qx, qy, trap);
    }

    // Random segments
    println("\n=== Random Segments (n=10) ===");
    auto rand_segs = random_segments(10, 50.0, 42);
    auto rand_decomp = build_trapezoidal(rand_segs);
    println("Built: {} trapezoids from {} segments",
            rand_decomp.trapezoids.size(), rand_decomp.segments.size());
    println("Verification: {}", verify_decomposition(rand_decomp) ? "PASS" : "FAIL");

    return 0;
}
