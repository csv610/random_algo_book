// example_chernoff_bound.cpp
// Demonstrates Chernoff bound formulas for tail probability estimation.
//
// The Chernoff bound provides exponentially decreasing upper bounds on the
// tail probabilities of sums of independent Bernoulli random variables.
// For X = X_1 + ... + X_n where X_i ~ Bernoulli(p_i) and mu = E[X]:
//   Upper tail: Pr[X >= (1+delta)*mu] <= (e^delta / (1+delta)^(1+delta))^mu
//   Lower tail: Pr[X <= (1-delta)*mu] <= (e^{-delta} / (1-delta)^{1-delta})^mu
//
// Simplified forms (for 0 < delta <= 1):
//   Upper: Pr[X >= (1+delta)*mu] <= exp(-mu * delta^2 / 3)
//   Lower: Pr[X <= (1-delta)*mu] <= exp(-mu * delta^2 / 2)

#include "ral/chernoff.h"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace ral;

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "=== Chernoff Bound Formulas ===\n\n";

    // Table of exact and simplified bounds for various mu, delta
    std::cout << "UPPER TAIL: Pr[X >= (1+delta)*mu]\n";
    std::cout << std::setw(8) << "mu"
              << std::setw(10) << "delta"
              << std::setw(14) << "Exact"
              << std::setw(14) << "Simplified"
              << std::setw(14) << "Ratio"
              << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (auto [mu, delta] : std::vector<std::pair<double,double>>{
        {5, 0.2}, {5, 0.5}, {10, 0.1}, {10, 0.3}, {10, 0.5},
        {20, 0.1}, {20, 0.2}, {50, 0.1}, {100, 0.05}, {100, 0.1}
    }) {
        double exact = chernoff_upper(mu, delta, true);
        double simp  = chernoff_simplified(mu, delta, true);
        std::cout << std::setw(8) << mu
                  << std::setw(10) << delta
                  << std::setw(14) << exact
                  << std::setw(14) << simp
                  << std::setw(14) << (simp / exact)
                  << "\n";
    }

    std::cout << "\nLOWER TAIL: Pr[X <= (1-delta)*mu]\n";
    std::cout << std::setw(8) << "mu"
              << std::setw(10) << "delta"
              << std::setw(14) << "Exact"
              << std::setw(14) << "Simplified"
              << std::setw(14) << "Ratio"
              << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (auto [mu, delta] : std::vector<std::pair<double,double>>{
        {5, 0.2}, {5, 0.4}, {10, 0.1}, {10, 0.3},
        {20, 0.1}, {20, 0.2}, {50, 0.1}, {100, 0.05}
    }) {
        double exact = chernoff_upper(mu, delta, false);
        double simp  = chernoff_simplified(mu, delta, false);
        std::cout << std::setw(8) << mu
                  << std::setw(10) << delta
                  << std::setw(14) << exact
                  << std::setw(14) << simp
                  << std::setw(14) << (simp / exact)
                  << "\n";
    }

    std::cout << "\nMOMENT GENERATING FUNCTION BOUNDS\n";
    std::cout << "E[e^{lambda X}] <= e^{mu(e^lambda - 1)}\n\n";
    std::cout << std::setw(8) << "mu"
              << std::setw(10) << "lambda"
              << std::setw(16) << "MGF Bound"
              << "\n";
    std::cout << std::string(34, '-') << "\n";

    for (auto [mu, lam] : std::vector<std::pair<double,double>>{
        {5, 0.1}, {5, 0.5}, {10, 0.2}, {10, 0.5}, {20, 0.1}, {20, 0.3}
    }) {
        std::cout << std::setw(8) << mu
                  << std::setw(10) << lam
                  << std::setw(16) << mgf_bound(mu, lam)
                  << "\n";
    }

    std::cout << "\nKey observations:\n";
    std::cout << "  - Bounds decrease exponentially in mu (concentration)\n";
    std::cout << "  - Simplified bound is tighter for small delta\n";
    std::cout << "  - Lower tail decays faster than upper tail\n";
    std::cout << "  - MGF bound is the starting point for deriving Chernoff bounds\n";

    return 0;
}
