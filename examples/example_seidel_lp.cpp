// =====================================================================
// Seidel's Randomized Linear Programming Solver
//
// Solves:  minimize  c^T x  subject to  a_i^T x <= b_i
//
// For fixed dimension d and n constraints, Seidel's algorithm runs
// in expected O(d! * n) time. The key idea:
//
//   1. Pick a random constraint i
//   2. Solve the LP without constraint i (recursively)
//   3. If the solution satisfies constraint i, return it
//   4. Otherwise, project onto the violated hyperplane (d-1 dim LP)
//
// This avoids the combinatorial explosion of checking constraint
// subsets. The random ordering gives a short expected recursion.
//
// This example solves several small LPs and verifies feasibility.
// =====================================================================

#include "ral/linear_programming.h"
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Seidel's Randomized LP Solver\n";
    std::cout << "=============================================\n\n";

    // --- Test 1: Simple 2D LP ---
    std::cout << "Test 1: Minimize x + y  s.t.  x + y <= 4,  x >= 0,  y >= 0\n";
    std::cout << "  Expected: optimal at (0, 0), objective = 0\n\n";
    {
        std::vector<std::vector<double>> A = {
            {-1.0, 0.0},
            {0.0, -1.0},
            {1.0, 1.0}
        };
        std::vector<double> b = {0.0, 0.0, 4.0};
        std::vector<double> c = {1.0, 1.0};

        auto res = solve_lp(A, b, c, 2);
        if (res.feasible)
            std::cout << "  x = (" << res.x[0] << ", " << res.x[1]
                      << "),  objective = " << res.objective << "\n\n";
        else
            std::cout << "  Infeasible\n\n";
    }

    // --- Test 2: Maximize x + y ---
    std::cout << "Test 2: Maximize x + y  s.t.  x + 2y <= 10,  2x + y <= 10,  x,y >= 0\n";
    std::cout << "  Expected: optimal near (10/3, 10/3)\n\n";
    {
        std::vector<std::vector<double>> A = {
            {1.0, 2.0},
            {2.0, 1.0},
            {-1.0, 0.0},
            {0.0, -1.0}
        };
        std::vector<double> b = {10.0, 10.0, 0.0, 0.0};
        std::vector<double> c = {-1.0, -1.0};  // minimize -x-y = maximize x+y

        auto res = solve_lp(A, b, c, 2);
        if (res.feasible)
            std::cout << "  x = (" << res.x[0] << ", " << res.x[1]
                      << "),  objective = " << res.objective
                      << "  (value = " << -res.objective << ")\n\n";
        else
            std::cout << "  Infeasible\n\n";
    }

    // --- Test 3: 3D LP ---
    std::cout << "Test 3: 3D LP — Minimize x + y + z\n";
    std::cout << "  s.t.  x+y+z >= 1,  x >= 0,  y >= 0,  z >= 0\n";
    std::cout << "  Expected: optimal at (1/3, 1/3, 1/3) or any point on x+y+z=1\n\n";
    {
        std::vector<std::vector<double>> A = {
            {-1.0, -1.0, -1.0},   // -(x+y+z) <= -1  => x+y+z >= 1
            {-1.0, 0.0, 0.0},     // -x <= 0
            {0.0, -1.0, 0.0},     // -y <= 0
            {0.0, 0.0, -1.0}      // -z <= 0
        };
        std::vector<double> b = {-1.0, 0.0, 0.0, 0.0};
        std::vector<double> c = {1.0, 1.0, 1.0};

        auto res = solve_lp(A, b, c, 3);
        if (res.feasible)
            std::cout << "  x = (" << res.x[0] << ", " << res.x[1] << ", " << res.x[2]
                      << "),  objective = " << res.objective << "\n\n";
        else
            std::cout << "  Infeasible\n\n";
    }

    // --- Test 4: Scaling ---
    std::cout << "Test 4: Random 2D LP with 1000 constraints\n";
    {
        std::mt19937 rng(42);
        std::uniform_real_distribution<double> dist(-10.0, 10.0);
        int n = 1000;
        std::vector<std::vector<double>> A(n, std::vector<double>(2));
        std::vector<double> b(n);
        for (int i = 0; i < n; i++) {
            A[i][0] = dist(rng);
            A[i][1] = dist(rng);
            b[i] = dist(rng) * 5.0;
        }
        std::vector<double> c = {1.0, -0.5};

        auto t0 = std::chrono::high_resolution_clock::now();
        auto res = solve_lp(A, b, c, 2);
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        if (res.feasible)
            std::cout << "  x = (" << res.x[0] << ", " << res.x[1]
                      << "),  objective = " << res.objective << "\n";
        else
            std::cout << "  Infeasible\n";
        std::cout << "  Time: " << std::fixed << std::setprecision(3) << ms << " ms\n\n";
    }

    std::cout << "Theory: Seidel's algorithm runs in O(d! * n) expected time.\n";
    return 0;
}
