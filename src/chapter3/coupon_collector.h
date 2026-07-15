#pragma once
// Coupon Collector Problem
// Theorem 3.8: Sharp threshold at n ln n

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace chapter3 {

// Simulate coupon collector: return number of trials to collect all n types
int simulate_coupon_collector(int n, std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, n - 1);
    std::vector<bool> collected(n, false);
    int remaining = n;
    int trials = 0;
    while (remaining > 0) {
        int coupon = dist(rng);
        ++trials;
        if (!collected[coupon]) {
            collected[coupon] = true;
            --remaining;
        }
    }
    return trials;
}

void demonstrate_coupon_collector() {
    std::cout << "Coupon Collector Problem (Section 3.6)\n\n";

    std::cout << "Experiment: Average trials vs n*H_n\n\n";
    std::cout << "  n      avg_trials   n*H_n     ratio\n";

    std::mt19937 rng(42);

    for (int n : {10, 50, 100, 500, 1000}) {
        double harmonic = 0.0;
        for (int i = 1; i <= n; ++i) harmonic += 1.0 / i;
        double expected_theory = n * harmonic;

        double total = 0;
        int trials_sim = 2000;
        for (int t = 0; t < trials_sim; ++t)
            total += simulate_coupon_collector(n, rng);
        double avg = total / trials_sim;

        std::cout << "  " << std::setw(4) << n
                  << "    " << std::setw(8) << std::fixed << std::setprecision(1) << avg
                  << "    " << std::setw(8) << expected_theory
                  << "   " << std::setw(5) << std::setprecision(2) << (avg / expected_theory) << "\n";
    }
    std::cout << std::defaultfloat << "\n";

    // Sharp threshold demonstration
    std::cout << "Theorem 3.8: Sharp Threshold\n";
    std::cout << "  Pr[X > n ln n + cn] -> 1 - e^{-e^{-c}} as n -> infinity\n\n";

    std::cout << "  c      theoretical   simulated (n=1000)\n";

    std::mt19937 rng2(42);
    int n = 1000;
    int trials_sharp = 5000;

    for (double c : {-2.0, -1.0, 0.0, 1.0, 2.0}) {
        double m = n * std::log(n) + c * n;
        int exceed = 0;
        for (int t = 0; t < trials_sharp; ++t) {
            std::uniform_int_distribution<int> dist(0, n - 1);
            std::vector<bool> seen(n, false);
            int rem = n;
            int tc = 0;
            while (rem > 0 && tc < static_cast<int>(m)) {
                int coupon = dist(rng2);
                ++tc;
                if (!seen[coupon]) { seen[coupon] = true; --rem; }
            }
            if (rem > 0) ++exceed;
        }
        double sim_prob = static_cast<double>(exceed) / trials_sharp;
        double theory_prob = 1.0 - std::exp(-std::exp(-c));
        std::cout << "  " << std::setw(4) << c << "    "
                  << std::setw(10) << theory_prob
                  << "        " << std::setw(10) << sim_prob << "\n";
    }
    std::cout << "\n";

    // Variance demonstration
    std::cout << "Variance of Coupon Collector:\n";
    std::cout << "  Var(X) <= pi^2 * n / 12\n\n";

    std::cout << "  n      sim_var    pi^2*n/12\n";
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
        double bound = M_PI * M_PI * n / 12.0;
        std::cout << "  " << std::setw(4) << n
                  << "    " << std::setw(8) << var
                  << "    " << std::setw(8) << bound << "\n";
    }
    std::cout << "\n";
}

}  // namespace chapter3
