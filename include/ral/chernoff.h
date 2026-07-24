#pragma once

#include <cmath>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <numeric>

namespace chapter5 {

struct ChernoffResult {
    double upper_bound;
    double exact_prob;
};

// Compute the Chernoff upper-tail bound: Pr[X >= (1+delta)*mu]
// using the exact form (e^delta / (1+delta)^(1+delta))^mu
// If upper=false, computes the lower-tail bound Pr[X <= (1-delta)*mu]
double chernoff_upper(double mu, double delta, bool upper = true) {
    if (mu <= 0) return 1.0;
    if (upper) {
        if (delta <= 0) return 1.0;
        double log_bound = mu * (delta - (1.0 + delta) * std::log(1.0 + delta));
        return std::exp(log_bound);
    } else {
        if (delta <= 0 || delta >= 1) return 1.0;
        double log_bound = mu * (-delta - (1.0 - delta) * std::log(1.0 - delta));
        return std::exp(log_bound);
    }
}

// Simplified Chernoff bound: Pr[X >= (1+delta)*mu] <= exp(-mu*delta^2/3)
// for 0 < delta <= 1, or exp(-mu*delta^2/2) for the lower tail
double chernoff_simplified(double mu, double delta, bool upper = true) {
    if (mu <= 0) return 1.0;
    if (delta <= 0 || delta > 1) {
        return chernoff_upper(mu, delta, upper);
    }
    if (upper) {
        return std::exp(-mu * delta * delta / 3.0);
    } else {
        return std::exp(-mu * delta * delta / 2.0);
    }
}

// Compute the moment generating function bound: E[e^{lambda X}] <= e^{mu(e^lambda - 1)}
// for X = sum of independent Bernoulli(p_i) with sum(p_i) = mu
double mgf_bound(double mu, double lambda) {
    return std::exp(mu * (std::exp(lambda) - 1.0));
}

// Simulate sum of n independent Bernoulli trials, each with probability p
// Return the observed sum
int simulate_bernoulli_sum(int n, double p, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (dist(rng) < p) count++;
    }
    return count;
}

// Estimate Pr[X >= (1+delta)*mu] by simulation
double simulate_upper_tail(int n, double p, double delta, int trials,
                           std::mt19937& rng) {
    double mu = n * p;
    int exceed = 0;
    for (int t = 0; t < trials; t++) {
        int x = simulate_bernoulli_sum(n, p, rng);
        if (x >= (1.0 + delta) * mu) exceed++;
    }
    return static_cast<double>(exceed) / trials;
}

// Estimate Pr[X <= (1-delta)*mu] by simulation
double simulate_lower_tail(int n, double p, double delta, int trials,
                           std::mt19937& rng) {
    double mu = n * p;
    int below = 0;
    for (int t = 0; t < trials; t++) {
        int x = simulate_bernoulli_sum(n, p, rng);
        if (x <= (1.0 - delta) * mu) below++;
    }
    return static_cast<double>(below) / trials;
}

// Demonstration: compare Chernoff bounds with simulated probabilities
void demonstrate_chernoff() {
    std::mt19937 rng(42);
    const int trials = 200000;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Chernoff Bounds: Theoretical Bounds vs. Simulation\n";
    std::cout << std::string(58, '-') << "\n\n";

    // Table: Upper tail
    std::cout << "UPPER TAIL: Pr[X >= (1+delta)*mu]\n";
    std::cout << std::setw(6) << "n"
              << std::setw(8) << "p"
              << std::setw(10) << "mu"
              << std::setw(8) << "delta"
              << std::setw(12) << "Simulated"
              << std::setw(12) << "Exact"
              << std::setw(12) << "Simplified"
              << "\n";
    std::cout << std::string(68, '-') << "\n";

    struct TestParam { int n; double p; double delta; };
    std::vector<TestParam> upper_tests = {
        {100, 0.5, 0.2}, {100, 0.5, 0.4}, {100, 0.1, 0.5},
        {200, 0.3, 0.2}, {200, 0.3, 0.3}, {500, 0.2, 0.2},
        {1000, 0.1, 0.15}, {1000, 0.01, 0.5}
    };

    for (const auto& t : upper_tests) {
        double mu = t.n * t.p;
        double exact = chernoff_upper(mu, t.delta, true);
        double simplified = chernoff_simplified(mu, t.delta, true);
        double simulated = simulate_upper_tail(t.n, t.p, t.delta, trials, rng);

        std::cout << std::setw(6) << t.n
                  << std::setw(8) << t.p
                  << std::setw(10) << mu
                  << std::setw(8) << t.delta
                  << std::setw(12) << simulated
                  << std::setw(12) << exact
                  << std::setw(12) << simplified
                  << "\n";
    }

    // Table: Lower tail
    std::cout << "\nLOWER TAIL: Pr[X <= (1-delta)*mu]\n";
    std::cout << std::setw(6) << "n"
              << std::setw(8) << "p"
              << std::setw(10) << "mu"
              << std::setw(8) << "delta"
              << std::setw(12) << "Simulated"
              << std::setw(12) << "Exact"
              << std::setw(12) << "Simplified"
              << "\n";
    std::cout << std::string(68, '-') << "\n";

    std::vector<TestParam> lower_tests = {
        {100, 0.5, 0.2}, {100, 0.5, 0.4}, {200, 0.3, 0.2},
        {200, 0.3, 0.3}, {500, 0.2, 0.2}, {1000, 0.1, 0.15}
    };

    for (const auto& t : lower_tests) {
        double mu = t.n * t.p;
        double exact = chernoff_upper(mu, t.delta, false);
        double simplified = chernoff_simplified(mu, t.delta, false);
        double simulated = simulate_lower_tail(t.n, t.p, t.delta, trials, rng);

        std::cout << std::setw(6) << t.n
                  << std::setw(8) << t.p
                  << std::setw(10) << mu
                  << std::setw(8) << t.delta
                  << std::setw(12) << simulated
                  << std::setw(12) << exact
                  << std::setw(12) << simplified
                  << "\n";
    }

    // MGF demonstration
    std::cout << "\n\nMoment Generating Function Bounds: E[e^{lambda X}] <= e^{mu(e^lambda - 1)}\n";
    std::cout << std::string(58, '-') << "\n";
    std::cout << std::setw(10) << "mu"
              << std::setw(10) << "lambda"
              << std::setw(16) << "MGF Bound"
              << "\n";
    std::cout << std::string(36, '-') << "\n";

    std::vector<std::pair<double, double>> mgf_tests = {
        {5.0, 0.1}, {5.0, 0.5}, {5.0, 1.0}, {10.0, 0.2},
        {10.0, 0.5}, {20.0, 0.1}, {20.0, 0.3}
    };

    for (const auto& [mu, lam] : mgf_tests) {
        std::cout << std::setw(10) << mu
                  << std::setw(10) << lam
                  << std::setw(16) << mgf_bound(mu, lam)
                  << "\n";
    }

    std::cout << "\nKey observations:\n";
    std::cout << "  - Exact and simplified bounds are close for moderate delta\n";
    std::cout << "  - Both bounds are valid upper bounds on the true probability\n";
    std::cout << "  - Bounds become tighter as mu increases (more concentration)\n";
    std::cout << "  - Lower tail decays faster than upper tail (exp(-mu*d^2/2) vs exp(-mu*d^2/3))\n";
}

} // namespace chapter5
