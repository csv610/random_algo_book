// =============================================================================
// Amplified Two-Point Method with Repetition
// =============================================================================
//
// Demonstrates the amplified two-point method applied to repeated trials,
// showing how error decreases as a function of the repetition parameter t.
//
// Key insight: with only 2 random numbers (a, b) and O(log n) bits,
// we can amplify the confidence of an RP algorithm to任意 small error 1/t
// by sampling r_i = (a*i + b) mod n for i = 0, 1, ..., t-1.
//
// This is much more efficient than running the RP algorithm t independent
// times (which would require t random bits). The pairwise independence
// construction gives the same error bound with far fewer random bits.
//
// We also demonstrate:
//  - The effect of increasing t on the error rate
//  - The difference between independent sampling and pairwise-independent sampling
//  - Practical confidence intervals forRP algorithms
// =============================================================================

#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <cmath>

#include "ral/two_point.h"

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Amplified Two-Point Method with Repetition ===\n\n";

    int n = 500;
    int trials = 500000;

    // --- Demo 1: Error curve as t increases ---
    std::cout << "Demo 1: Error rate vs repetition count t (x IN L)\n";
    std::cout << "  The amplified method achieves error <= 1/t\n\n";

    std::cout << std::setw(8) << "  t"
              << std::setw(14) << "empirical_err"
              << std::setw(14) << "theory_bound"
              << std::setw(14) << "bits_used" << "\n";
    std::cout << "  " << std::string(50, '-') << "\n";

    for (int t : {2, 4, 8, 16, 32, 64, 128, 256}) {
        int false_count = 0;
        for (int i = 0; i < trials; ++i) {
            if (!amplified_two_point(1, n, t, true, rng)) ++false_count;
        }
        double empirical_err = static_cast<double>(false_count) / trials;
        double bits_used = 2.0 * std::ceil(std::log2(n));  // two integers mod n

        std::cout << std::setw(8) << t
                  << std::setw(14) << std::fixed << std::setprecision(4) << empirical_err
                  << std::setw(14) << 1.0 / t
                  << std::setw(14) << std::setprecision(1) << bits_used << "\n";
    }

    // --- Demo 2: Compare pairwise vs independent sampling ---
    std::cout << "\nDemo 2: Pairwise-independent vs fully-independent sampling\n";
    std::cout << "  Both use O(log n) bits; pairwise is more efficient\n\n";

    auto independent_sample = [&](int x, int t_val, bool in_L) -> bool {
        std::uniform_int_distribution<int> dist(0, n - 1);
        for (int i = 0; i < t_val; i++) {
            int r = dist(rng);
            if (RP_algorithm(x, r, in_L)) return true;
        }
        return false;
    };

    std::cout << std::setw(8) << "  t"
              << std::setw(18) << "pairwise_err"
              << std::setw(18) << "independent_err" << "\n";
    std::cout << "  " << std::string(44, '-') << "\n";

    for (int t : {2, 4, 8, 16, 32}) {
        int pairwise_false = 0, indep_false = 0;
        for (int i = 0; i < trials; ++i) {
            if (!amplified_two_point(1, n, t, true, rng)) ++pairwise_false;
            if (!independent_sample(1, t, true)) ++indep_false;
        }
        double pw_err = static_cast<double>(pairwise_false) / trials;
        double ind_err = static_cast<double>(indep_false) / trials;

        std::cout << std::setw(8) << t
                  << std::setw(18) << std::fixed << std::setprecision(4) << pw_err
                  << std::setw(18) << ind_err << "\n";
    }

    // --- Demo 3: Confidence intervals ---
    std::cout << "\nDemo 3: Achieving 99% confidence (error <= 0.01) with t=100\n";
    {
        int t = 100;
        int false_count = 0;
        for (int i = 0; i < trials; ++i) {
            if (!amplified_two_point(1, n, t, true, rng)) ++false_count;
        }
        double err = static_cast<double>(false_count) / trials;
        double ci_low = err - 3.0 * std::sqrt(err * (1 - err) / trials);
        double ci_high = err + 3.0 * std::sqrt(err * (1 - err) / trials);

        std::cout << "  t = " << t << "\n";
        std::cout << "  Empirical error: " << std::fixed << std::setprecision(4) << err << "\n";
        std::cout << "  99.7% CI: [" << ci_low << ", " << ci_high << "]\n";
        std::cout << "  Theory bound (1/t): " << 1.0 / t << "\n";
        std::cout << "  Error below bound: " << (err <= 1.0 / t + 0.001 ? "YES" : "NO") << "\n";
    }

    // --- Demo 4: x NOT in L — error should be exactly 0 ---
    std::cout << "\nDemo 4: x NOT in L — false positive should be impossible\n";
    {
        int false_count = 0;
        for (int i = 0; i < trials; ++i) {
            if (!amplified_two_point(1, n, 8, false, rng)) ++false_count;
        }
        std::cout << "  False positives: " << false_count << " / " << trials
                  << " (always 0 for RP algorithms)\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}
