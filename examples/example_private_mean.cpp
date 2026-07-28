// =====================================================================
// Private Mean Computation with Differential Privacy
//
// Computes the mean of a dataset while satisfying epsilon-differential
// privacy. The approach:
//   1. Compute the true sum of all values
//   2. Add Laplace noise calibrated to the sensitivity of the sum
//      (sensitivity = 2 * value_bound, since one value can change
//       from 0 to value_bound or vice versa)
//   3. Divide the noisy sum by n to get the noisy mean
//
// This achieves (epsilon)-DP because division by a constant doesn't
// change the privacy guarantee.
//
// This example computes private average salaries with different
// privacy budgets and demonstrates the accuracy tradeoff.
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
    std::cout << "  Private Mean Computation (DP)\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    // Generate salary dataset
    int n = 5000;
    double value_bound = 200000.0;  // max salary
    std::vector<double> salaries(n);
    std::normal_distribution<double> dist(65000.0, 20000.0);
    for (int i = 0; i < n; ++i) {
        salaries[i] = std::clamp(dist(rng), 20000.0, value_bound);
    }

    double true_sum = 0.0;
    for (double v : salaries) true_sum += v;
    double true_mean = true_sum / n;

    std::cout << "Dataset: " << n << " salaries\n";
    std::cout << "Value bound: $" << std::fixed << std::setprecision(0)
              << value_bound << "\n";
    std::cout << "True mean salary: $" << true_mean << "\n\n";

    // Single release with different epsilon values
    std::cout << "--- Single Release Accuracy ---\n";
    std::cout << std::setw(10) << "Epsilon"
              << std::setw(16) << "Private Mean"
              << std::setw(14) << "Error"
              << std::setw(12) << "Rel Error%"
              << "\n";

    for (double eps : {0.1, 0.5, 1.0, 2.0, 5.0}) {
        double noisy = private_mean(salaries, value_bound, eps, rng);
        double error = std::abs(noisy - true_mean);
        double rel = 100.0 * error / true_mean;
        std::cout << std::setw(10) << std::setprecision(1) << eps
                  << std::setw(16) << std::setprecision(0) << noisy
                  << std::setw(14) << std::setprecision(0) << error
                  << std::setw(12) << std::setprecision(2) << rel
                  << "\n";
    }

    // Statistical analysis over many releases
    std::cout << "\n--- Statistical Analysis (1000 releases per epsilon) ---\n";
    std::cout << std::setw(10) << "Epsilon"
              << std::setw(14) << "Mean Error"
              << std::setw(14) << "Std Dev"
              << std::setw(14) << "95th pctl"
              << "\n";

    for (double eps : {0.1, 0.5, 1.0, 5.0}) {
        std::vector<double> errors;
        for (int t = 0; t < 1000; ++t) {
            double noisy = private_mean(salaries, value_bound, eps, rng);
            errors.push_back(noisy - true_mean);
        }

        double mean_err = 0.0;
        for (double e : errors) mean_err += e;
        mean_err /= errors.size();

        double variance = 0.0;
        for (double e : errors) variance += (e - mean_err) * (e - mean_err);
        double stddev = std::sqrt(variance / errors.size());

        std::sort(errors.begin(), errors.end());
        double p95 = errors[static_cast<int>(0.95 * errors.size())];

        std::cout << std::setw(10) << std::setprecision(1) << eps
                  << std::setw(14) << std::setprecision(0) << std::abs(mean_err)
                  << std::setw(14) << std::setprecision(0) << stddev
                  << std::setw(14) << std::setprecision(0) << std::abs(p95)
                  << "\n";
    }

    // Show the sensitivity calculation
    std::cout << "\n--- Sensitivity Analysis ---\n";
    std::cout << "Sum sensitivity: 2 * value_bound = $"
              << std::setprecision(0) << 2.0 * value_bound << "\n";
    std::cout << "Laplace scale for epsilon=1.0: $"
              << 2.0 * value_bound / 1.0 << "\n";
    std::cout << "Mean noise = sum noise / " << n << "\n";
    std::cout << "Expected mean error for epsilon=1.0: ~$"
              << 2.0 * value_bound / (1.0 * n) << "\n";

    std::cout << "\nPrivate mean: add Lap(2B/epsilon) to sum, divide by n.\n";
    std::cout << "Larger datasets dilute the noise: error ~ O(B/(epsilon*n)).\n";

    return 0;
}
