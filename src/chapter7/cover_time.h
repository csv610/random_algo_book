#pragma once

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "random_walk.h"

namespace chapter7 {

inline int simulate_cover_time(const std::vector<std::vector<int>>& adj,
                               std::mt19937& rng) {
    int n = static_cast<int>(adj.size());
    std::vector<bool> visited(n, false);
    int cur = 0;
    int remaining = n;
    int steps = 0;

    while (remaining > 0) {
        cur = random_walk_step(cur, adj, rng);
        steps++;
        if (!visited[cur]) {
            visited[cur] = true;
            remaining--;
        }
    }
    return steps;
}

inline int simulate_hitting_time(const std::vector<std::vector<int>>& adj,
                                 int start, int target, std::mt19937& rng,
                                 int max_steps = 10000000) {
    int cur = start;
    for (int t = 0; t < max_steps; ++t) {
        if (cur == target) return t;
        cur = random_walk_step(cur, adj, rng);
    }
    return max_steps;
}

inline double estimate_hitting_time(const std::vector<std::vector<int>>& adj,
                                    int target, int trials, std::mt19937& rng) {
    double total = 0.0;
    for (int i = 0; i < trials; ++i) {
        int start = 0;
        while (start == target)
            start = std::uniform_int_distribution<int>(
                0, static_cast<int>(adj.size()) - 1)(rng);
        total += simulate_hitting_time(adj, start, target, rng);
    }
    return total / trials;
}

inline void demonstrate_cover_time() {
    std::cout << "Cover Times and Hitting Times\n\n";

    std::mt19937 rng(42);

    {
        std::cout << "--- Star Graph K_{1,9} (n=10) ---\n";
        auto adj = make_star(10);

        const int trials = 2000;
        double total_cover = 0.0;
        for (int i = 0; i < trials; ++i) {
            std::mt19937 local_rng(rng());
            total_cover += simulate_cover_time(adj, local_rng);
        }
        double avg_cover = total_cover / trials;
        double theory = 2.0 * 9.0 * (10.0 - 1.0);

        std::cout << "  Simulated cover time: " << std::fixed << std::setprecision(1)
                  << avg_cover << "\n";
        std::cout << "  Theorem 6.2 upper bound (2m(n-1)): " << theory << "\n";
        std::cout << "  Matthews bound (H_max * H_{n-1}): ~"
                  << (9.0 * 9.0) * (1.0 + 1.0/2.0 + 1.0/3.0 + 1.0/4.0 +
                                      1.0/5.0 + 1.0/6.0 + 1.0/7.0 + 1.0/8.0 + 1.0/9.0)
                  << "\n";

        double ht_center_leaf = 0.0;
        for (int i = 0; i < 500; ++i) {
            ht_center_leaf += simulate_hitting_time(adj, 0, 1, rng);
        }
        double ht_leaf_center = 0.0;
        for (int i = 0; i < 500; ++i) {
            ht_leaf_center += simulate_hitting_time(adj, 1, 0, rng);
        }
        std::cout << "  H(center, leaf): " << ht_center_leaf / 500.0 << "\n";
        std::cout << "  H(leaf, center): " << ht_leaf_center / 500.0 << "\n";
        std::cout << "  Commute time:    " << (ht_center_leaf + ht_leaf_center) / 500.0 << "\n";
        std::cout << "  2m*R_eff (theory): " << 2.0 * 9.0 * 1.0 << "\n\n";
    }

    {
        std::cout << "--- Cycle C_10 (n=10) ---\n";
        auto adj = make_cycle(10);

        const int trials = 2000;
        double total_cover = 0.0;
        for (int i = 0; i < trials; ++i) {
            std::mt19937 local_rng(rng());
            total_cover += simulate_cover_time(adj, local_rng);
        }
        double avg_cover = total_cover / trials;
        double theory = 2.0 * 10.0 * 9.0;

        std::cout << "  Simulated cover time: " << std::fixed << std::setprecision(1)
                  << avg_cover << "\n";
        std::cout << "  Theorem 6.2 upper bound (2m(n-1)): " << theory << "\n";
        std::cout << "  Theta(n^2) = 100 (for C_n)\n\n";
    }

    {
        std::cout << "--- Complete Graph K_8 (n=8) ---\n";
        auto adj = make_complete(8);

        const int trials = 2000;
        double total_cover = 0.0;
        for (int i = 0; i < trials; ++i) {
            std::mt19937 local_rng(rng());
            total_cover += simulate_cover_time(adj, local_rng);
        }
        double avg_cover = total_cover / trials;

        double harmonic = 0.0;
        for (int i = 1; i <= 7; ++i) harmonic += 1.0 / i;

        std::cout << "  Simulated cover time: " << std::fixed << std::setprecision(1)
                  << avg_cover << "\n";
        std::cout << "  Coupon collector (n * H_{n-1}): " << 8.0 * harmonic << "\n";
        std::cout << "  Theorem 6.2 upper bound: " << 2.0 * 28.0 * 7.0 << "\n\n";
    }

    {
        std::cout << "--- Hitting Time Comparison (n=8) ---\n";
        auto adj = make_cycle(8);

        double h_0_4 = 0.0;
        for (int i = 0; i < 1000; ++i)
            h_0_4 += simulate_hitting_time(adj, 0, 4, rng);
        std::cout << "  Cycle C_8: H(0,4) = " << std::fixed << std::setprecision(1)
                  << h_0_4 / 1000.0 << "\n";

        auto comp = make_complete(8);
        double h_comp = 0.0;
        for (int i = 0; i < 1000; ++i)
            h_comp += simulate_hitting_time(comp, 0, 1, rng);
        std::cout << "  Complete K_8: H(0,1) = " << h_comp / 1000.0 << "\n";
        std::cout << "  (Theory: n-1 = 7 for complete graph)\n\n";
    }
}

} // namespace chapter7
