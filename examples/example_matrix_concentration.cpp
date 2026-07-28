// =============================================================================
// Matrix Concentration Inequalities (Chernoff / Bernstein Bounds)
//
// For a sum of independent random matrices X = X_1 + X_2 + ... + X_n
// where each X_i is a symmetric d x d matrix with E[X_i] = 0 and
// ||X_i|| <= R almost surely, the matrix Chernoff bound gives:
//
//   Pr[||X|| >= t] <= d * exp(-t^2 / (2 * n * R^2))    (Gaussian case)
//
// Matrix Bernstein:
//   Pr[||X|| >= t] <= 2d * exp(-t^2 / (2 * sigma^2 + 2*Rt/3))
//   where sigma^2 = ||sum E[X_i^2]||.
//
// This example demonstrates the scalar Chernoff bound for sums of
// Bernoulli random variables and compares empirical vs theoretical bounds.
// =============================================================================

#include "ral/chernoff.h"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <iomanip>
#include <algorithm>

namespace ral {

inline void demonstrate_concentration() {
    std::mt19937 rng(42);

    std::cout << "=== Matrix/Scalar Concentration Inequalities ===\n\n";

    // --- Scalar Chernoff bounds ---
    std::cout << "--- Chernoff Bounds: Pr[X >= (1+delta)*mu] ---\n\n";

    int n = 100;
    double p = 0.5;
    double mu = n * p;  // = 50

    std::cout << "Setting: X = sum of " << n << " Bernoulli(" << p << "),  mu = " << mu << "\n\n";

    std::cout << std::left
              << std::setw(10) << "delta"
              << std::setw(18) << "Threshold"
              << std::setw(18) << "Exact Chernoff"
              << std::setw(18) << "Simplified"
              << std::setw(18) << "Simulated" << "\n";
    std::cout << std::string(82, '-') << "\n";

    for (double delta : {0.1, 0.2, 0.3, 0.5, 1.0}) {
        double threshold = (1.0 + delta) * mu;

        // Exact Chernoff bound
        double exact = chernoff_upper(mu, delta);

        // Simplified bound
        double simplified = chernoff_simplified(mu, delta);

        // Empirical: simulate many trials
        int num_trials = 100000;
        int exceed = 0;
        for (int t = 0; t < num_trials; ++t) {
            int sum = simulate_bernoulli_sum(n, p, rng);
            if (sum >= threshold) exceed++;
        }
        double empirical = static_cast<double>(exceed) / num_trials;

        std::cout << std::setw(10) << std::fixed << std::setprecision(2) << delta
                  << std::setw(18) << std::fixed << std::setprecision(1) << threshold
                  << std::setw(18) << std::scientific << std::setprecision(4) << exact
                  << std::setw(18) << std::scientific << std::setprecision(4) << simplified
                  << std::setw(18) << std::scientific << std::setprecision(4) << empirical << "\n";
    }

    // --- Lower tail ---
    std::cout << "\n--- Lower Tail: Pr[X <= (1-delta)*mu] ---\n\n";
    std::cout << std::left
              << std::setw(10) << "delta"
              << std::setw(18) << "Threshold"
              << std::setw(18) << "Chernoff"
              << std::setw(18) << "Simulated" << "\n";
    std::cout << std::string(64, '-') << "\n";

    for (double delta : {0.1, 0.2, 0.3, 0.5}) {
        double threshold = (1.0 - delta) * mu;
        double bound = chernoff_upper(mu, delta, false);

        int num_trials = 100000;
        int below = 0;
        for (int t = 0; t < num_trials; ++t) {
            int sum = simulate_bernoulli_sum(n, p, rng);
            if (sum <= threshold) below++;
        }
        double empirical = static_cast<double>(below) / num_trials;

        std::cout << std::setw(10) << std::fixed << std::setprecision(2) << delta
                  << std::setw(18) << std::fixed << std::setprecision(1) << threshold
                  << std::setw(18) << std::scientific << std::setprecision(4) << bound
                  << std::setw(18) << std::scientific << std::setprecision(4) << empirical << "\n";
    }

    // --- Matrix Chernoff illustration (scalar proxy) ---
    std::cout << "\n--- Matrix Chernoff: Spectral Norm of Random Matrices ---\n\n";
    std::cout << "Simulating X = sum of n random symmetric 3x3 matrices\n";
    std::cout << "with entries in [-1, 1]. Using matrix Chernoff bound.\n\n";

    int mat_n = 50;
    int d = 3;
    int trials = 10000;
    std::uniform_real_distribution<double> unif(-1.0, 1.0);

    std::vector<double> spectral_norms;
    for (int t = 0; t < trials; ++t) {
        // Accumulate random symmetric matrices
        std::vector<std::vector<double>> X(d, std::vector<double>(d, 0.0));
        for (int k = 0; k < mat_n; ++k) {
            std::vector<std::vector<double>> M(d, std::vector<double>(d));
            for (int i = 0; i < d; ++i)
                for (int j = i; j < d; ++j) {
                    M[i][j] = unif(rng);
                    M[j][i] = M[i][j];
                }
            for (int i = 0; i < d; ++i)
                for (int j = 0; j < d; ++j)
                    X[i][j] += M[i][j];
        }
        // Approximate spectral norm via power iteration
        std::vector<double> v(d, 1.0 / std::sqrt(d));
        for (int iter = 0; iter < 20; ++iter) {
            std::vector<double> w(d, 0.0);
            for (int i = 0; i < d; ++i)
                for (int j = 0; j < d; ++j)
                    w[i] += X[i][j] * v[j];
            double norm = 0;
            for (int i = 0; i < d; ++i) norm += w[i] * w[i];
            norm = std::sqrt(norm);
            for (int i = 0; i < d; ++i) v[i] = w[i] / norm;
        }
        double lambda = 0;
        for (int i = 0; i < d; ++i) {
            double val = 0;
            for (int j = 0; j < d; ++j) val += v[i] * X[i][j] * v[j];
            lambda += val;
        }
        spectral_norms.push_back(std::abs(lambda));
    }

    // Empirical percentiles
    std::sort(spectral_norms.begin(), spectral_norms.end());
    auto pct = [&](double frac) { return spectral_norms[(int)(frac * trials)]; };

    std::cout << "  n = " << mat_n << " matrices, d = " << d << "\n";
    std::cout << "  Empirical spectral norms ||sum X_i||:\n";
    std::cout << "    50th percentile: " << std::fixed << std::setprecision(2) << pct(0.50) << "\n";
    std::cout << "    90th percentile: " << std::fixed << std::setprecision(2) << pct(0.90) << "\n";
    std::cout << "    95th percentile: " << std::fixed << std::setprecision(2) << pct(0.95) << "\n";
    std::cout << "    99th percentile: " << std::fixed << std::setprecision(2) << pct(0.99) << "\n";

    // Theoretical matrix Chernoff bound (simplified)
    // For X_i symmetric with ||X_i|| <= R, E[X_i] = 0:
    // Pr[||sum X_i|| >= t] <= d * exp(-t^2 / (2*n*R^2))
    double R = 2.0;  // spectral norm bound for symmetric 3x3 with entries in [-1,1]
    double t_val = pct(0.95);
    double mcb = d * std::exp(-t_val * t_val / (2.0 * mat_n * R * R));
    std::cout << "\n  Matrix Chernoff bound at t = " << std::fixed << std::setprecision(2) << t_val
              << ":  Pr[||X|| >= t] <= " << std::scientific << std::setprecision(4) << mcb << "\n";
    std::cout << "  (empirical: 5% exceed)\n";
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_concentration();
    return 0;
}
