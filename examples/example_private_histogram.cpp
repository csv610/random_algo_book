// =====================================================================
// Private Histogram with Per-Bucket Laplace Noise
//
// Releases a histogram (frequency count per bucket) while satisfying
// epsilon-differential privacy. The approach:
//   1. Count the number of records in each bucket (exact counts)
//   2. Split the privacy budget: epsilon_total / num_buckets per bucket
//   3. Add independent Laplace noise to each bucket count
//
// Sensitivity of each bucket count is 1 (one record moves at most
// one count). The per-bucket noise scale is 1 / (epsilon / num_buckets).
//
// This example builds a private histogram of age distribution with
// different privacy budgets and compares against ground truth.
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
    std::cout << "  Private Histogram (Per-Bucket Laplace)\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    // Generate age dataset
    int n = 5000;
    int num_buckets = 10;  // decades: 0-9, 10-19, ..., 90-99
    std::vector<int> ages(n);
    std::normal_distribution<double> dist(40.0, 15.0);
    for (int i = 0; i < n; ++i) {
        ages[i] = std::clamp(static_cast<int>(dist(rng)), 0, 99);
    }

    // True histogram
    std::vector<double> true_hist(num_buckets, 0.0);
    for (int a : ages) true_hist[a / 10]++;

    std::cout << "Dataset: " << n << " people, " << num_buckets << " age buckets\n\n";

    // Display true histogram
    std::cout << "--- True Age Distribution ---\n";
    std::cout << std::setw(12) << "Age Group"
              << std::setw(10) << "Count"
              << std::setw(10) << "Percent"
              << "\n";
    for (int i = 0; i < num_buckets; ++i) {
        std::cout << std::setw(12) << (std::to_string(i * 10) + "-" + std::to_string(i * 10 + 9))
                  << std::setw(10) << std::setprecision(0) << true_hist[i]
                  << std::setw(10) << std::setprecision(1)
                  << 100.0 * true_hist[i] / n
                  << "\n";
    }

    // Release private histogram with different epsilon values
    for (double eps : {0.5, 1.0, 5.0}) {
        std::cout << "\n--- Private Histogram (epsilon=" << eps
                  << ", per-bucket eps=" << std::setprecision(3)
                  << eps / num_buckets << ") ---\n";

        // Single release
        auto private_hist = private_histogram(ages, num_buckets, eps, rng);

        std::cout << std::setw(12) << "Age Group"
                  << std::setw(10) << "True"
                  << std::setw(12) << "Private"
                  << std::setw(10) << "Error"
                  << "\n";

        double total_abs_error = 0.0;
        for (int i = 0; i < num_buckets; ++i) {
            double error = private_hist[i] - true_hist[i];
            total_abs_error += std::abs(error);
            std::cout << std::setw(12) << (std::to_string(i * 10) + "-" + std::to_string(i * 10 + 9))
                      << std::setw(10) << std::setprecision(0) << true_hist[i]
                      << std::setw(12) << std::setprecision(1) << private_hist[i]
                      << std::setw(10) << std::setprecision(1) << error
                      << "\n";
        }
        std::cout << "Total absolute error: " << std::setprecision(0)
                  << total_abs_error << "\n";
    }

    // Statistical analysis over many releases
    std::cout << "\n--- Accuracy Over 500 Releases ---\n";
    std::cout << std::setw(10) << "Epsilon"
              << std::setw(16) << "Avg Total Err"
              << std::setw(16) << "Avg Max Err"
              << std::setw(16) << "Avg L2 Err"
              << "\n";

    for (double eps : {0.5, 1.0, 2.0, 5.0}) {
        double total_ae = 0.0, total_me = 0.0, total_l2 = 0.0;
        int trials = 500;
        for (int t = 0; t < trials; ++t) {
            auto h = private_histogram(ages, num_buckets, eps, rng);
            double ae = 0.0, me = 0.0, l2 = 0.0;
            for (int i = 0; i < num_buckets; ++i) {
                double err = h[i] - true_hist[i];
                ae += std::abs(err);
                me = std::max(me, std::abs(err));
                l2 += err * err;
            }
            total_ae += ae;
            total_me += me;
            total_l2 += std::sqrt(l2);
        }
        std::cout << std::setw(10) << std::setprecision(1) << eps
                  << std::setw(16) << std::setprecision(1) << total_ae / trials
                  << std::setw(16) << std::setprecision(1) << total_me / trials
                  << std::setw(16) << std::setprecision(1) << total_l2 / trials
                  << "\n";
    }

    std::cout << "\nPrivate histogram splits epsilon across buckets:\n";
    std::cout << "  Per-bucket noise scale = 1 / (epsilon / num_buckets)\n";
    std::cout << "  More buckets or smaller epsilon = noisier per bucket.\n";
    std::cout << "  Tradeoff: granularity vs accuracy.\n";

    return 0;
}
