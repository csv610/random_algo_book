#pragma once
// Occupancy Problems: Balls-in-bins and Birthday Problem
// Theorem 3.1: Max occupancy is O((en ln n)/(ln ln n)) w.h.p.

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace chapter3 {

struct OccupancyResult {
    int max_load;
    int empty_bins;
    std::vector<int> histogram;  // histogram[k] = #bins with exactly k balls
};

// Throw m balls into n bins uniformly at random
OccupancyResult simulate_occupancy(int m, int n, std::mt19937& rng) {
    std::vector<int> bins(n, 0);
    std::uniform_int_distribution<int> dist(0, n - 1);

    for (int i = 0; i < m; ++i)
        ++bins[dist(rng)];

    OccupancyResult result;
    result.max_load = 0;
    result.empty_bins = 0;
    int max_observed = 0;
    for (int b : bins) {
        max_observed = std::max(max_observed, b);
        if (b == 0) ++result.empty_bins;
    }
    result.histogram.resize(max_observed + 1, 0);
    for (int b : bins)
        ++result.histogram[b];
    result.max_load = max_observed;
    return result;
}

// Birthday problem: probability of collision among m balls in n bins
double birthday_collision_prob(int m, int n, int trials, std::mt19937& rng) {
    int collisions = 0;
    std::uniform_int_distribution<int> dist(0, n - 1);
    for (int t = 0; t < trials; ++t) {
        std::vector<bool> seen(n, false);
        bool collision = false;
        for (int i = 0; i < m; ++i) {
            int bday = dist(rng);
            if (seen[bday]) { collision = true; break; }
            seen[bday] = true;
        }
        if (collision) ++collisions;
    }
    return static_cast<double>(collisions) / trials;
}

void demonstrate_occupancy() {
    std::cout << "Occupancy Problems (Section 3.1)\n\n";

    std::cout << "Experiment 1: m = n balls into n bins\n";
    std::cout << "  Theorem 3.1: max load <= (en ln n)/(ln ln n) w.h.p.\n\n";

    std::cout << "  n     max_load  empty_bins  theoretical_max\n";
    for (int n : {100, 500, 1000, 5000, 10000}) {
        std::mt19937 rng(42);
        double total_max = 0;
        int trials = 100;
        for (int t = 0; t < trials; ++t) {
            auto r = simulate_occupancy(n, n, rng);
            total_max += r.max_load;
        }
        double avg_max = total_max / trials;
        double theory = (std::exp(1.0) * std::log(n)) / std::log(std::log(n));
        std::cout << "  " << n << "    "
                  << std::setw(8) << avg_max << "    "
                  << std::setw(5) << (n * std::exp(-1.0)) << "     "
                  << std::setw(8) << theory << "\n";
    }
    std::cout << "\n";

    std::cout << "Experiment 2: Birthday Problem\n";
    std::cout << "  How many people before 50% chance of shared birthday?\n";
    std::cout << "  Theory: m ~= sqrt(2n ln 2) ~= 22.49 for n=365\n\n";

    std::cout << "  m     simulated   theoretical\n";
    for (int m : {10, 15, 20, 23, 30, 50}) {
        std::mt19937 rng(42);
        double sim = birthday_collision_prob(m, 365, 100000, rng);
        double theory = 1.0 - std::exp(-static_cast<double>(m * (m - 1)) / (2.0 * 365));
        std::cout << "  " << std::setw(2) << m << "    "
                  << std::setw(8) << std::fixed << std::setprecision(4) << sim
                  << "     " << std::setw(8) << theory << "\n";
    }
    std::cout << std::defaultfloat << "\n\n";
}

}  // namespace chapter3
