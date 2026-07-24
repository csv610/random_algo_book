#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <chrono>

namespace chapter10 {

// Seidel's randomized LP algorithm (1991)
// Solves:  minimize  c^T x  subject to  a_i^T x <= b_i  for i = 1..n
// where the number of constraints n can be large but dimension d is fixed.
// Expected running time: O(d! * n) for fixed d.

bool satisfies(const std::vector<double>& x,
               const std::vector<double>& a, double b, double eps = 1e-9) {
    double val = 0.0;
    for (size_t i = 0; i < x.size(); i++) {
        val += a[i] * x[i];
    }
    return val <= b + eps;
}

// Solve a system of k equations in d variables via Gaussian elimination.
// Returns the solution vector of length d.
std::vector<double> solve_equality_system(
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& b, int d) {
    int k = static_cast<int>(A.size());
    if (k == 0) return std::vector<double>(d, 0.0);

    // Build augmented matrix [A | b]
    std::vector<std::vector<double>> mat(k, std::vector<double>(d + 1, 0.0));
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < d; j++) mat[i][j] = A[i][j];
        mat[i][d] = b[i];
    }

    int row = 0;
    for (int col = 0; col < d && row < k; col++) {
        // Find pivot
        int pivot = row;
        for (int r = row + 1; r < k; r++) {
            if (std::abs(mat[r][col]) > std::abs(mat[pivot][col])) pivot = r;
        }
        std::swap(mat[row], mat[pivot]);
        if (std::abs(mat[row][col]) < 1e-12) continue;

        // Eliminate
        for (int r = 0; r < k; r++) {
            if (r == row) continue;
            double f = mat[r][col] / mat[row][col];
            for (int j = col; j <= d; j++) mat[r][j] -= f * mat[row][j];
        }
        row++;
    }

    std::vector<double> x(d, 0.0);
    row = 0;
    for (int col = 0; col < d && row < k; col++) {
        if (std::abs(mat[row][col]) > 1e-12) {
            x[col] = mat[row][d] / mat[row][col];
        }
        row++;
    }
    return x;
}

// Seidel's randomized LP in d dimensions (recursive).
// constraints is a list of (a, b) pairs for a^T x <= b.
std::vector<double> seidel_lp_rec(
    const std::vector<std::pair<std::vector<double>, double>>& constraints,
    const std::vector<double>& c, int d, std::mt19937& rng) {

    int n = static_cast<int>(constraints.size());

    // Base case: d or fewer constraints, solve via equalities
    if (n <= d) {
        if (n == 0) return std::vector<double>(d, 0.0);
        // Solve a_i^T x = b_i for i = 0..n-1, minimize c^T x
        std::vector<std::vector<double>> A_eq(n, std::vector<double>(d));
        std::vector<double> b_eq(n);
        for (int i = 0; i < n; i++) {
            A_eq[i] = constraints[i].first;
            b_eq[i] = constraints[i].second;
        }
        return solve_equality_system(A_eq, b_eq, d);
    }

    // Pick a random constraint
    std::uniform_int_distribution<int> dist(0, n - 1);
    int idx = dist(rng);
    auto picked = constraints[idx];

    // Build new constraint set without the picked constraint
    std::vector<std::pair<std::vector<double>, double>> reduced;
    reduced.reserve(n - 1);
    for (int i = 0; i < n; i++) {
        if (i != idx) reduced.push_back(constraints[i]);
    }

    // Solve without the picked constraint
    auto x = seidel_lp_rec(reduced, c, d, rng);

    // Check if the picked constraint is satisfied
    if (!x.empty() && satisfies(x, picked.first, picked.second)) {
        return x;  // constraint is redundant
    }

    // Constraint is violated: project onto its boundary a^T x = b
    // This becomes an (d-1)-dimensional LP on the hyperplane
    const auto& a = picked.first;
    double b_val = picked.second;

    // Choose pivot variable: the one with largest |a[j]|
    int pivot_j = 0;
    double max_abs = 0.0;
    for (int j = 0; j < d; j++) {
        if (std::abs(a[j]) > max_abs) {
            max_abs = std::abs(a[j]);
            pivot_j = j;
        }
    }
    if (max_abs < 1e-12) return x;  // degenerate constraint

    // Substitute: x_pivot = (b - sum_{j != pivot} a[j]*x[j]) / a[pivot]
    int new_d = d - 1;
    double a_pivot = a[pivot_j];

    // Build (d-1)-dimensional constraints
    std::vector<std::pair<std::vector<double>, double>> projected;
    for (int ci = 0; ci < n - 1; ci++) {
        const auto& ai = reduced[ci].first;
        double bi = reduced[ci].second;

        // New constraint in (d-1) variables:
        // For each j != pivot_j, the coefficient becomes ai[j] - ai[pivot_j]*a[j]/a[pivot]
        // The RHS becomes bi - ai[pivot_j]*b/a[pivot]
        std::vector<double> a_new(new_d);
        for (int j = 0; j < d; j++) {
            if (j == pivot_j) continue;
            int nj = (j < pivot_j) ? j : j - 1;
            a_new[nj] = ai[j] - ai[pivot_j] * a[j] / a_pivot;
        }
        double b_new = bi - ai[pivot_j] * b_val / a_pivot;
        projected.push_back({a_new, b_new});
    }

    // New objective in (d-1) variables
    std::vector<double> c_new(new_d);
    for (int j = 0; j < d; j++) {
        if (j == pivot_j) continue;
        int nj = (j < pivot_j) ? j : j - 1;
        c_new[nj] = c[j] - c[pivot_j] * a[j] / a_pivot;
    }

    // Recurse on the projected LP
    auto x2 = seidel_lp_rec(projected, c_new, new_d, rng);

    // Reconstruct full-dimensional solution
    std::vector<double> full_x(d, 0.0);
    double sum_ax = 0.0;
    for (int j = 0; j < d; j++) {
        if (j == pivot_j) continue;
        int nj = (j < pivot_j) ? j : j - 1;
        full_x[j] = x2[nj];
        sum_ax += a[j] * x2[nj];
    }
    full_x[pivot_j] = (b_val - sum_ax) / a_pivot;

    return full_x;
}

// Main entry point for Seidel's randomized LP
std::vector<double> randomized_lp(const std::vector<std::vector<double>>& A,
                                   const std::vector<double>& b,
                                   const std::vector<double>& c,
                                   int d) {
    int n = static_cast<int>(A.size());
    std::random_device rd;
    std::mt19937 rng(rd());

    std::vector<std::pair<std::vector<double>, double>> constraints(n);
    for (int i = 0; i < n; i++) {
        constraints[i] = {A[i], b[i]};
    }

    return seidel_lp_rec(constraints, c, d, rng);
}

struct LPResult {
    std::vector<double> x;
    double objective;
    bool feasible;
};

LPResult solve_lp(const std::vector<std::vector<double>>& A,
                  const std::vector<double>& b,
                  const std::vector<double>& c, int d) {
    LPResult result;
    result.x = randomized_lp(A, b, c, d);
    result.feasible = !result.x.empty();
    result.objective = 0.0;
    if (result.feasible) {
        for (int j = 0; j < d; j++) {
            result.objective += c[j] * result.x[j];
        }
    }
    return result;
}

void demonstrate_lp() {
    std::random_device rd;
    std::mt19937 rng(rd());

    std::cout << "  Test 1: Minimize x + y subject to:\n";
    std::cout << "    x + y <= 4,  x >= 0,  y >= 0\n";
    std::cout << "    Expected: optimal at (0, 0), objective = 0\n";

    {
        std::vector<std::vector<double>> A = {
            {-1.0, 0.0},
            {0.0, -1.0},
            {1.0, 1.0}
        };
        std::vector<double> b = {0.0, 0.0, 4.0};
        std::vector<double> c = {1.0, 1.0};

        auto res = solve_lp(A, b, c, 2);
        if (res.feasible) {
            std::cout << "  Result: x = (" << res.x[0] << ", " << res.x[1]
                      << "), objective = " << res.objective << "\n";
        } else {
            std::cout << "  Result: infeasible\n";
        }
    }

    std::cout << "\n  Test 2: Minimize -x - y (maximize x + y) subject to:\n";
    std::cout << "    x + 2y <= 10,  2x + y <= 10,  x,y >= 0\n";
    std::cout << "    Expected: optimal near (10/3, 10/3)\n";

    {
        std::vector<std::vector<double>> A = {
            {1.0, 2.0},
            {2.0, 1.0},
            {-1.0, 0.0},
            {0.0, -1.0}
        };
        std::vector<double> b = {10.0, 10.0, 0.0, 0.0};
        std::vector<double> c = {-1.0, -1.0};

        auto res = solve_lp(A, b, c, 2);
        if (res.feasible) {
            std::cout << "  Result: x = (" << res.x[0] << ", " << res.x[1]
                      << "), objective = " << res.objective << "\n";
        } else {
            std::cout << "  Result: infeasible\n";
        }
    }

    std::cout << "\n  Test 3: Random 2D LP with 1000 constraints\n";

    {
        int n = 1000;
        std::uniform_real_distribution<double> coeff_dist(-10.0, 10.0);
        std::vector<std::vector<double>> A(n, std::vector<double>(2));
        std::vector<double> b(n);
        for (int i = 0; i < n; i++) {
            A[i][0] = coeff_dist(rng);
            A[i][1] = coeff_dist(rng);
            b[i] = coeff_dist(rng) * 5.0;
        }
        std::vector<double> c = {1.0, -0.5};

        auto t_start = std::chrono::high_resolution_clock::now();
        auto res = solve_lp(A, b, c, 2);
        auto t_end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(t_end - t_start).count();

        if (res.feasible) {
            std::cout << "  Result: x = (" << res.x[0] << ", " << res.x[1]
                      << "), objective = " << res.objective << "\n";
        } else {
            std::cout << "  Result: infeasible\n";
        }
        std::cout << "  Time: " << std::fixed << std::setprecision(3) << elapsed << " ms\n";
    }

    std::cout << "\n  Test 4: Random 3D LP with 200 constraints\n";

    {
        int n = 200;
        std::uniform_real_distribution<double> coeff_dist(-10.0, 10.0);
        std::vector<std::vector<double>> A(n, std::vector<double>(3));
        std::vector<double> b(n);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 3; j++) A[i][j] = coeff_dist(rng);
            b[i] = coeff_dist(rng) * 5.0;
        }
        std::vector<double> c = {1.0, -2.0, 0.5};

        auto t_start = std::chrono::high_resolution_clock::now();
        auto res = solve_lp(A, b, c, 3);
        auto t_end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(t_end - t_start).count();

        if (res.feasible) {
            std::cout << "  Result: x = (" << res.x[0] << ", " << res.x[1]
                      << ", " << res.x[2] << "), objective = " << res.objective << "\n";
        } else {
            std::cout << "  Result: infeasible\n";
        }
        std::cout << "  Time: " << std::fixed << std::setprecision(3) << elapsed << " ms\n";
    }
}

}  // namespace chapter10
