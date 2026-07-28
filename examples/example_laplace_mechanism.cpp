// =====================================================================
// Laplace Mechanism for Differential Privacy
//
// For a numeric query f with global sensitivity Delta_f, the mechanism
//   M(x) = f(x) + Lap(Delta_f / epsilon)
// satisfies epsilon-differential privacy.
//
// The Laplace distribution has heavier tails than Gaussian, providing
// exactly epsilon-DP. Smaller epsilon = more privacy = more noise.
//
// This example demonstrates the privacy-utility tradeoff by releasing
// the mean income of a dataset with varying epsilon values and measuring
// the resulting estimation error.
// =====================================================================

#include "ral/differential_privacy.h"
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Laplace Mechanism: Differential Privacy\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    // Generate synthetic income dataset
    int n = 10000;
    std::vector<double> incomes(n);
    std::normal_distribution<double> dist(50000.0, 15000.0);
    for (int i = 0; i < n; ++i) {
        incomes[i] = std::max(10000.0, dist(rng));
    }

    double true_sum = 0.0;
    for (double v : incomes) true_sum += v;
    double true_mean = true_sum / n;
    double value_bound = 100000.0;  // max possible income

    std::cout << "Dataset: " << n << " incomes\n";
    std::cout << "True mean income: $" << std::fixed << std::setprecision(0)
              << true_mean << "\n\n";

    // Demonstrate Laplace noise properties
    std::cout << "--- Laplace Noise Samples (epsilon=1.0, sensitivity=1.0) ---\n";
    std::cout << "  scale = sensitivity/epsilon = 1.0\n";
    for (int i = 0; i < 10; ++i) {
        double sample = laplace_sample(1.0, rng);
        std::cout << "  " << std::setw(2) << i + 1 << ": " << std::setw(10)
                  << std::setprecision(4) << sample << "\n";
    }

    // Privacy-utility tradeoff
    std::cout << "\n--- Privacy-Utility Tradeoff ---\n";
    std::cout << std::setw(10) << "Epsilon"
              << std::setw(18) << "Avg Abs Error"
              << std::setw(14) << "Relative%"
              << std::setw(16) << "Privacy Level"
              << "\n";

    for (double eps : {0.1, 0.2, 0.5, 1.0, 2.0, 5.0, 10.0}) {
        int trials = 1000;
        double total_error = 0.0;
        for (int t = 0; t < trials; ++t) {
            double noisy_mean = private_mean(incomes, value_bound, eps, rng);
            total_error += std::abs(noisy_mean - true_mean);
        }
        double avg_error = total_error / trials;
        double rel_error = 100.0 * avg_error / true_mean;

        std::string privacy;
        if (eps <= 0.1) privacy = "Very Strong";
        else if (eps <= 0.5) privacy = "Strong";
        else if (eps <= 1.0) privacy = "Moderate";
        else if (eps <= 5.0) privacy = "Weak";
        else privacy = "Very Weak";

        std::cout << std::setw(10) << std::setprecision(1) << eps
                  << std::setw(18) << std::setprecision(0) << avg_error
                  << std::setw(14) << std::setprecision(2) << rel_error
                  << std::setw(16) << privacy
                  << "\n";
    }

    // Count query with Laplace mechanism
    std::cout << "\n--- Private Count Query ---\n";
    std::cout << "Query: How many incomes exceed $60,000?\n";

    // Exact count
    int exact_count = 0;
    for (double v : incomes) {
        if (v > 60000.0) exact_count++;
    }
    std::cout << "True count: " << exact_count << "\n";

    // Private count with different epsilon
    for (double eps : {0.5, 1.0, 5.0}) {
        double total = 0.0;
        for (int t = 0; t < 1000; ++t) {
            std::vector<bool> database(n);
            for (int i = 0; i < n; ++i) database[i] = (incomes[i] > 60000.0);
            total += private_count(database, [](int) { return true; }, eps, rng);
        }
        double avg = total / 1000.0;
        std::cout << "  epsilon=" << std::setprecision(1) << eps
                  << ": avg estimate=" << std::setprecision(0) << avg
                  << " (error=" << std::setprecision(1) << std::abs(avg - exact_count) << ")\n";
    }

    std::cout << "\nLaplace mechanism: M(x) = f(x) + Lap(Delta/epsilon)\n";
    std::cout << "Sensitivity Delta = max |f(D) - f(D')| over neighboring datasets.\n";
    std::cout << "Smaller epsilon = more noise = stronger privacy guarantee.\n";

    return 0;
}
