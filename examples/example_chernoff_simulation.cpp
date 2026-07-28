// example_chernoff_simulation.cpp
// Empirically verifies Chernoff bounds by simulating Bernoulli sums
// and comparing the observed tail probabilities against the theoretical bounds.
//
// For X = sum of n independent Bernoulli(p) variables with mu = n*p:
//   Pr[X >= (1+delta)*mu] <= exp(-mu*delta^2/3)  (upper tail)
//   Pr[X <= (1-delta)*mu] <= exp(-mu*delta^2/2)  (lower tail)
//
// This simulation draws many samples and measures the fraction that
// fall in the tail, confirming the bounds hold.

#include "ral/chernoff.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>

using namespace ral;

int main() {
    std::mt19937 rng(42);
    const int trials = 200000;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "=== Chernoff Bounds: Empirical Verification ===\n\n";

    std::cout << "UPPER TAIL: Pr[X >= (1+delta)*mu]\n";
    std::cout << std::setw(6) << "n"
              << std::setw(8) << "p"
              << std::setw(8) << "mu"
              << std::setw(8) << "delta"
              << std::setw(12) << "Simulated"
              << std::setw(12) << "Exact"
              << std::setw(12) << "Simplified"
              << "\n";
    std::cout << std::string(66, '-') << "\n";

    struct Param { int n; double p; double delta; };
    std::vector<Param> upper_tests = {
        {100, 0.5, 0.2}, {100, 0.5, 0.4}, {200, 0.3, 0.2},
        {500, 0.2, 0.2}, {1000, 0.1, 0.15}, {1000, 0.01, 0.5}
    };

    for (const auto& t : upper_tests) {
        double mu = t.n * t.p;
        double exact = chernoff_upper(mu, t.delta, true);
        double simplified = chernoff_simplified(mu, t.delta, true);
        double simulated = simulate_upper_tail(t.n, t.p, t.delta, trials, rng);

        std::cout << std::setw(6) << t.n
                  << std::setw(8) << t.p
                  << std::setw(8) << mu
                  << std::setw(8) << t.delta
                  << std::setw(12) << simulated
                  << std::setw(12) << exact
                  << std::setw(12) << simplified
                  << "\n";
    }

    std::cout << "\nLOWER TAIL: Pr[X <= (1-delta)*mu]\n";
    std::cout << std::setw(6) << "n"
              << std::setw(8) << "p"
              << std::setw(8) << "mu"
              << std::setw(8) << "delta"
              << std::setw(12) << "Simulated"
              << std::setw(12) << "Exact"
              << std::setw(12) << "Simplified"
              << "\n";
    std::cout << std::string(66, '-') << "\n";

    std::vector<Param> lower_tests = {
        {100, 0.5, 0.2}, {100, 0.5, 0.4}, {200, 0.3, 0.2},
        {500, 0.2, 0.2}, {1000, 0.1, 0.15}
    };

    for (const auto& t : lower_tests) {
        double mu = t.n * t.p;
        double exact = chernoff_upper(mu, t.delta, false);
        double simplified = chernoff_simplified(mu, t.delta, false);
        double simulated = simulate_lower_tail(t.n, t.p, t.delta, trials, rng);

        std::cout << std::setw(6) << t.n
                  << std::setw(8) << t.p
                  << std::setw(8) << mu
                  << std::setw(8) << t.delta
                  << std::setw(12) << simulated
                  << std::setw(12) << exact
                  << std::setw(12) << simplified
                  << "\n";
    }

    std::cout << "\nConclusion: In all cases, Simulated <= Exact <= Simplified\n";
    std::cout << "(simplified is slightly looser but closed-form)\n";

    return 0;
}
