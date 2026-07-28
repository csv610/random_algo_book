// =============================================================================
// Two-Point Method for Set Balance (Probability Amplification)
// =============================================================================
//
// The two-point sampling technique amplifies the correctness of a
// randomized RP algorithm from error <= 1/4 to error <= 1/t using
// only O(log n) random bits via pairwise independence.
//
// Given an RP algorithm A(x, r) where:
//   - If x is in the language L: Pr_r[A(x,r) = 1] >= 1/2
//   - If x is NOT in L:          Pr_r[A(x,r) = 0] = 1
//
// Naive two-point: pick random a, b. Return A(x,a) OR A(x,b).
//   Error <= (1/2)^2 = 1/4.
//
// Amplified version: pick random a, b. For i = 0..t-1:
//   Compute r_i = (a*i + b) mod n, and return A(x, r_i) for any i.
//   By pairwise independence, error <= 1/t.
//
// This achieves exponential error reduction with only 2 random numbers.
// =============================================================================

#include <iostream>
#include <vector>
#include <random>
#include <iomanip>

#include "ral/two_point.h"

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Two-Point Method for Set Balance ===\n\n";

    std::cout << "Goal: Amplify RP error from 1/4 to 1/t\n";
    std::cout << "Method: Use pairwise-independent sampling r_i = (a*i + b) mod n\n";
    std::cout << "Random bits used: only 2 (a and b), regardless of t\n\n";

    // --- Demo 1: Error rates for different t ---
    std::cout << "Demo 1: Error rates for x IN L (should almost never reject)\n\n";

    int n = 1000;
    int trials = 200000;

    std::cout << std::setw(6) << "  t"
              << std::setw(16) << "naive_error"
              << std::setw(18) << "amplified_error"
              << std::setw(12) << "1/t_bound" << "\n";
    std::cout << "  " << std::string(52, '-') << "\n";

    for (int t : {2, 4, 8, 16, 32, 64, 128}) {
        int naive_false = 0, amp_false = 0;
        for (int i = 0; i < trials; ++i) {
            if (!naive_two_point(1, n, true, rng)) ++naive_false;
            if (!amplified_two_point(1, n, t, true, rng)) ++amp_false;
        }
        double naive_err = static_cast<double>(naive_false) / trials;
        double amp_err   = static_cast<double>(amp_false) / trials;

        std::cout << std::setw(6) << t
                  << std::setw(16) << std::fixed << std::setprecision(4) << naive_err
                  << std::setw(18) << amp_err
                  << std::setw(12) << std::setprecision(4) << (1.0 / t) << "\n";
    }

    // --- Demo 2: Error rates for x NOT in L (should always reject) ---
    std::cout << "\nDemo 2: Error rates for x NOT in L (should always be 0)\n\n";

    std::cout << std::setw(6) << "  t"
              << std::setw(16) << "naive_error"
              << std::setw(18) << "amplified_error" << "\n";
    std::cout << "  " << std::string(40, '-') << "\n";

    for (int t : {2, 8, 32, 128}) {
        int naive_false = 0, amp_false = 0;
        for (int i = 0; i < trials; ++i) {
            if (!naive_two_point(1, n, false, rng)) ++naive_false;
            if (!amplified_two_point(1, n, t, false, rng)) ++amp_false;
        }
        double naive_err = static_cast<double>(naive_false) / trials;
        double amp_err   = static_cast<double>(amp_false) / trials;

        std::cout << std::setw(6) << t
                  << std::setw(16) << std::fixed << std::setprecision(4) << naive_err
                  << std::setw(18) << amp_err << "\n";
    }

    // --- Demo 3: Pairwise independence verification ---
    std::cout << "\nDemo 3: Pairwise independence of r_i = (a*i + b) mod n\n";
    demonstrate_pairwise_independence(7, 42);

    std::cout << "Done.\n";
    return 0;
}
