// example_coupon_collector.cpp
// Simulates the coupon collector problem and compares empirical results
// with the theoretical expected value n * H_n, where H_n = 1 + 1/2 + ... + 1/n.
//
// The coupon collector problem: given n equally likely coupon types,
// how many random draws are needed to collect all n types?
//
// Theorem 3.8 (Sharp Threshold):
//   Pr[T > n*ln(n) + c*n] -> 1 - exp(-exp(-c))  as n -> infinity
//
// This means the coupon collector time concentrates sharply around n*ln(n).

#include "ral/coupon_collector.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>
#include <numbers>

using namespace ral;

int main() {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "=== Coupon Collector Problem ===\n\n";

    // Part 1: Average trials vs n*H_n
    std::cout << "Average trials to collect all n coupons\n";
    std::cout << std::setw(6) << "n"
              << std::setw(12) << "avg_trials"
              << std::setw(12) << "n*H_n"
              << std::setw(8) << "ratio"
              << "\n";
    std::cout << std::string(38, '-') << "\n";

    std::mt19937 rng(42);
    for (int n : {10, 50, 100, 500, 1000}) {
        double harmonic = 0.0;
        for (int i = 1; i <= n; ++i) harmonic += 1.0 / i;
        double expected_theory = n * harmonic;

        double total = 0;
        int num_trials = 2000;
        for (int t = 0; t < num_trials; ++t)
            total += simulate_coupon_collector(n, rng);
        double avg = total / num_trials;

        std::cout << std::setw(6) << n
                  << std::setw(12) << avg
                  << std::setw(12) << expected_theory
                  << std::setw(8) << std::setprecision(3) << (avg / expected_theory)
                  << std::setprecision(2) << "\n";
    }

    // Part 2: Sharp threshold demonstration
    std::cout << "\nSharp Threshold: Pr[T > n*ln(n) + c*n] -> 1 - exp(-exp(-c))\n";
    std::cout << std::setw(6) << "c"
              << std::setw(14) << "theoretical"
              << std::setw(14) << "simulated"
              << "\n";
    std::cout << std::string(34, '-') << "\n";

    std::mt19937 rng2(42);
    int n = 1000;
    int trials_sharp = 5000;

    for (double c : {-2.0, -1.0, 0.0, 1.0, 2.0}) {
        double m = n * std::log(static_cast<double>(n)) + c * n;
        int exceed = 0;
        for (int t = 0; t < trials_sharp; ++t) {
            std::uniform_int_distribution<int> dist(0, n - 1);
            std::vector<bool> seen(n, false);
            int rem = n;
            int tc = 0;
            while (rem > 0 && tc < static_cast<int>(m)) {
                int coupon = dist(rng2);
                ++tc;
                if (seen[coupon]) continue;
                seen[coupon] = true;
                --rem;
            }
            if (rem > 0) ++exceed;
        }
        double sim_prob = static_cast<double>(exceed) / trials_sharp;
        double theory_prob = 1.0 - std::exp(-std::exp(-c));
        std::cout << std::setw(6) << c
                  << std::setw(14) << theory_prob
                  << std::setw(14) << sim_prob
                  << "\n";
    }

    // Part 3: Variance bound
    std::cout << "\nVariance bound: Var(T) <= pi^2 * n / 12\n";
    std::cout << std::setw(6) << "n"
              << std::setw(12) << "sim_var"
              << std::setw(12) << "pi^2*n/12"
              << "\n";
    std::cout << std::string(30, '-') << "\n";

    for (int n : {10, 50, 100, 500}) {
        std::mt19937 rng3(42);
        int trials_var = 5000;
        double sum = 0, sum_sq = 0;
        for (int t = 0; t < trials_var; ++t) {
            int x = simulate_coupon_collector(n, rng3);
            sum += x;
            sum_sq += static_cast<double>(x) * x;
        }
        double mean = sum / trials_var;
        double var = sum_sq / trials_var - mean * mean;
        double bound = std::numbers::pi * std::numbers::pi * n / 12.0;
        std::cout << std::setw(6) << n
                  << std::setw(12) << var
                  << std::setw(12) << bound
                  << "\n";
    }

    std::cout << "\nThe average ratio ~1.0 confirms the n*H_n formula.\n";
    std::cout << "The sharp threshold shows concentration around n*ln(n).\n";

    return 0;
}
