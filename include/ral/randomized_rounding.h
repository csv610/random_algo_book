#pragma once

#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>
#include "random_utils.h"
#include "linear_programming.h"

namespace ral {

// ---------------------------------------------------------------------------
// Randomized Rounding
// Technique for converting fractional LP/IP solutions to integer solutions.
// Given a fractional solution x* in [0,1]^n, round each coordinate
// independently: x_i = 1 with probability x*_i, else 0.
//
// If the LP constraints have the form sum(a_ij * x_j) <= b_i with
// a_ij >= 0, then E[sum(a_ij * x_j)] = sum(a_ij * x*_j) <= b_i,
// and by Markov's inequality, Pr[sum(a_ij * x_j) > t * b_i] <= 1/t.
// ---------------------------------------------------------------------------

struct RoundingResult {
    std::vector<int> rounded;
    double lp_objective;
    int rounded_objective;
    bool satisfies_constraints;
};

// Basic randomized rounding: each x_i rounded independently
inline RoundingResult randomized_rounding_basic(
    const std::vector<double>& lp_solution,
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& b,
    const std::vector<double>& c,
    std::mt19937& rng) {

    int n = static_cast<int>(lp_solution.size());
    int m = static_cast<int>(A.size());

    // Round each variable independently
    std::vector<int> rounded(n);
    std::uniform_real_distribution<double> unif(0.0, 1.0);
    for (int i = 0; i < n; ++i) {
        rounded[i] = (unif(rng) < lp_solution[i]) ? 1 : 0;
    }

    // Check constraints: sum_j A[i][j] * rounded[j] <= b[i]
    bool feasible = true;
    for (int i = 0; i < m; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            sum += A[i][j] * rounded[j];
        }
        if (sum > b[i] + 1e-9) {
            feasible = false;
            break;
        }
    }

    // Compute objective
    double lp_obj = 0.0;
    int rounded_obj = 0;
    for (int j = 0; j < n; ++j) {
        lp_obj += c[j] * lp_solution[j];
        rounded_obj += static_cast<int>(c[j] * rounded[j]);
    }

    return {rounded, lp_obj, rounded_obj, feasible};
}

// Randomized rounding with pipage rounding for monotone submodular maximization
// Repeatedly rounds fractional solution, taking the best among multiple trials
inline RoundingResult randomized_rounding_best_of(
    const std::vector<double>& lp_solution,
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& b,
    const std::vector<double>& c,
    int trials, std::mt19937& rng) {

    RoundingResult best = randomized_rounding_basic(
        lp_solution, A, b, c, rng);

    for (int t = 1; t < trials; ++t) {
        auto current = randomized_rounding_basic(lp_solution, A, b, c, rng);

        // Prefer feasible solutions, then higher objective
        if (current.satisfies_constraints && !best.satisfies_constraints) {
            best = current;
        } else if (current.satisfies_constraints == best.satisfies_constraints) {
            if (current.rounded_objective > best.rounded_objective) {
                best = current;
            }
        }
    }
    return best;
}

// Chernoff-concentration-aware rounding
// For constraints of the form sum(a_ij * x_j) <= b_i with a_ij in [0,1],
// use the Chernoff bound: Pr[S > (1+d)*mu] <= exp(-d^2*mu/3)
inline RoundingResult chernoff_rounding(
    const std::vector<double>& lp_solution,
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& b,
    const std::vector<double>& c,
    int trials, std::mt19937& rng) {

    int n = static_cast<int>(lp_solution.size());
    int m = static_cast<int>(A.size());

    // Repeat until we find a feasible solution, up to trials attempts
    for (int t = 0; t < trials; ++t) {
        std::vector<int> rounded(n);
        std::uniform_real_distribution<double> unif(0.0, 1.0);
        for (int i = 0; i < n; ++i) {
            rounded[i] = (unif(rng) < lp_solution[i]) ? 1 : 0;
        }

        bool feasible = true;
        for (int i = 0; i < m; ++i) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) {
                sum += A[i][j] * rounded[j];
            }
            if (sum > b[i] + 1e-9) {
                feasible = false;
                break;
            }
        }

        if (feasible) {
            double obj = 0.0;
            int int_obj = 0;
            for (int j = 0; j < n; ++j) {
                obj += c[j] * lp_solution[j];
                int_obj += static_cast<int>(c[j] * rounded[j]);
            }
            return {rounded, obj, int_obj, true};
        }
    }

    // Fallback to basic rounding
    return randomized_rounding_basic(lp_solution, A, b, c, rng);
}

// Demonstration
inline void demonstrate_randomized_rounding() {
    std::mt19937 rng(42);

    println("=== Randomized Rounding ===\n");

    // Example: Set Cover relaxation
    // Minimize sum(x_j) subject to: for each set i, sum_{j in S_i} x_j >= 1
    // LP relaxation: x_j in [0,1]
    // Rounding: x_j = 1 with probability x*_j

    println("--- Example: Weighted Set Cover Rounding ---");

    // 6 elements, 4 sets
    // Sets: S0={0,1}, S1={1,2,3}, S2={3,4}, S3={0,4,5}
    // LP minimizes cost: 2*x0 + 3*x1 + 1*x2 + 2*x3
    int n = 4;  // number of sets (variables)
    std::vector<double> c = {2.0, 3.0, 1.0, 2.0};

    // Coverage constraints (transposed): for each element, sum sets covering it >= 1
    // Formulated as <= for our LP solver: -sum >= -1 -> sum >= 1
    std::vector<std::vector<double>> A = {
        {-1.0,  0.0,  0.0, -1.0},   // element 0: covered by sets 0, 3
        {-1.0, -1.0,  0.0,  0.0},   // element 1: covered by sets 0, 1
        { 0.0, -1.0,  0.0,  0.0},   // element 2: covered by set 1
        { 0.0, -1.0, -1.0,  0.0},   // element 3: covered by sets 1, 2
        { 0.0,  0.0, -1.0, -1.0},   // element 4: covered by sets 2, 3
        { 0.0,  0.0,  0.0, -1.0},   // element 5: covered by set 3
    };
    std::vector<double> b = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0};

    // Solve LP relaxation (negate objective for minimization)
    std::vector<double> c_neg = {-2.0, -3.0, -1.0, -2.0};
    auto res = solve_lp(A, b, c_neg, n);

    if (res.feasible) {
        println("  LP optimal solution:");
        print("    x* = (");
        for (int j = 0; j < n; ++j) {
            if (j) print(", ");
            print("{:.3f}", -res.x[j]);
        }
        println(")");
        println("  LP objective (min cost) = {:.3f}", -res.objective);

        // Run randomized rounding many times
        int good = 0;
        double best_int_obj = 1e18;
        for (int t = 0; t < 10000; ++t) {
            auto rounded = randomized_rounding_basic(res.x, A, b, c, rng);
            if (rounded.satisfies_constraints) good++;

            int cost = 0;
            for (int j = 0; j < n; ++j) {
                cost += static_cast<int>(c[j] * rounded.rounded[j]);
            }
            if (cost < best_int_obj) best_int_obj = cost;
        }

        println("  Randomized rounding (10,000 trials):");
        println("    Fraction feasible: {:.1f}%", 100.0 * good / 10000);
        println("    Best integer cost found: {:.0f}", best_int_obj);
        println("    Integrality gap: {:.2f}x", best_int_obj / (-res.objective));
    }

    // Second example: simple fractional knapsack rounding
    println("\n--- Simple LP Rounding ---");
    {
        // Maximize 3x + 5y subject to: x + 2y <= 4, x,y in [0,1]
        std::vector<std::vector<double>> A2 = {
            {1.0, 2.0},
            {-1.0, 0.0},
            {0.0, -1.0}
        };
        std::vector<double> b2 = {4.0, 0.0, 0.0};
        std::vector<double> c2 = {-3.0, -5.0};

        auto res2 = solve_lp(A2, b2, c2, 2);
        println("  Maximize 3x + 5y subject to x + 2y <= 4, 0 <= x,y <= 1");
        if (res2.feasible) {
            println("  LP optimum: ({:.3f}, {:.3f}), obj = {:.3f}",
                          -res2.x[0], -res2.x[1], -res2.objective);

            // All 4 possible integer solutions
            println("  Integer solutions:");
            for (int x = 0; x <= 1; ++x) {
                for (int y = 0; y <= 1; ++y) {
                    if (x + 2 * y <= 4) {
                        println("    ({}, {}): objective = {}", x, y, 3 * x + 5 * y);
                    }
                }
            }
        }
    }
}

} // namespace ral
