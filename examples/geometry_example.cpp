#include "ral.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================================\n";
    std::cout << "           RAL Geometry & LP Examples\n";
    std::cout << "========================================================\n\n";

    // ----------------------------------------------------------------
    // 1. Convex Hull Algorithms
    // ----------------------------------------------------------------
    std::cout << "--- 1. Convex Hull (Randomized Incremental vs Jarvis) ---\n";
    std::vector<ral::Point2D> points = {
        {0.0, 0.0}, {4.0, 0.0}, {4.0, 4.0}, {0.0, 4.0},
        {1.0, 1.0}, {2.0, 2.0}, {3.0, 1.0}, {1.5, 3.0}
    };

    auto hull_ric = ral::convex_hull(points);
    auto hull_jarvis = ral::gift_wrap_hull(points);

    std::cout << "  Input points size: " << points.size() << "\n";
    std::cout << "  Convex Hull (RIC) vertices: ";
    for (const auto& p : hull_ric) {
        std::cout << "(" << p.x << ", " << p.y << ") ";
    }
    std::cout << "\n";

    std::cout << "  Convex Hull (Jarvis) vertices: ";
    for (const auto& p : hull_jarvis) {
        std::cout << "(" << p.x << ", " << p.y << ") ";
    }
    std::cout << "\n\n";

    // ----------------------------------------------------------------
    // 2. Delaunay Triangulation
    // ----------------------------------------------------------------
    std::cout << "--- 2. Delaunay Triangulation (Bowyer-Watson) ---\n";
    std::vector<ral::Point2D> dt_points = {
        {0.0, 0.0}, {4.0, 0.0}, {0.0, 4.0}, {4.0, 4.0}, {2.0, 2.0}
    };
    auto triangles = ral::delaunay_triangulation(dt_points);
    std::cout << "  Triangulation generated " << triangles.size() << " triangles:\n";
    for (size_t i = 0; i < triangles.size(); ++i) {
        std::cout << "    Triangle " << i << ": indices ["
                  << triangles[i].v[0] << ", "
                  << triangles[i].v[1] << ", "
                  << triangles[i].v[2] << "]\n";
    }
    std::cout << "\n";

    // ----------------------------------------------------------------
    // 3. Seidel's Linear Programming Solver
    // ----------------------------------------------------------------
    std::cout << "--- 3. Seidel's Randomized LP Solver ---\n";
    // We solve:
    // minimize: -2x - y (objective c = {-2, -1})
    // subject to:
    //   -x <= 0  ==> x >= 0
    //   -y <= 0  ==> y >= 0
    //    x + y <= 4
    //    2x + y <= 6
    std::vector<std::vector<double>> A = {
        {-1.0, 0.0},
        {0.0, -1.0},
        {1.0, 1.0},
        {2.0, 1.0}
    };
    std::vector<double> b = {0.0, 0.0, 4.0, 6.0};
    std::vector<double> c = {-2.0, -1.0};
    int dim = 2;

    auto lp_result = ral::solve_lp(A, b, c, dim);

    std::cout << "  LP Feasible: " << (lp_result.feasible ? "Yes" : "No") << "\n";
    if (lp_result.feasible) {
        std::cout << "  Optimal solution x*: (" << lp_result.x[0] << ", " << lp_result.x[1] << ")\n";
        std::cout << "  Optimal objective value: " << lp_result.objective << " (Expected: -6.0)\n";
    }
    std::cout << "\n";

    // ----------------------------------------------------------------
    // 4. Binary Planar Partitions (BSP)
    // ----------------------------------------------------------------
    std::cout << "--- 4. Binary Planar Partitions ---\n";
    std::vector<ral::Segment> segments = {
        {0.0, 0.0, 2.0, 2.0, 1},
        {2.0, 0.0, 0.0, 2.0, 2},
        {1.0, 0.0, 1.0, 2.0, 3}
    };

    auto root = ral::rand_auto_partition(segments);
    int size = ral::partition_size(root);
    std::cout << "  Constructed BSP size (including splits): " << size << "\n";
    delete root; // clean up memory

    std::cout << "\n========================================================\n\n";
    return 0;
}
