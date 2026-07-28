// example_monte_carlo_pi.cpp
// Estimates pi by Monte Carlo sampling: pick random points in [-1,1]^2
// and count how many fall inside the unit circle. The ratio approximates
// pi/4.
//
// This is a Monte Carlo algorithm with two-sided error (BPP-style):
//   Pr[|estimate - pi| > epsilon] <= 2*exp(-2*n*epsilon^2/pi^2)
// by the Chernoff bound.
//
// The error decreases as O(1/sqrt(n)) by the CLT.

#include "ral/las_vegas_monte_carlo.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <numbers>

using namespace ral;

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "=== Monte Carlo Estimation of Pi ===\n\n";

    std::cout << "Method: sample random points in [-1,1]^2,\n";
    std::cout << "count those inside unit circle, ratio -> pi/4\n\n";

    std::cout << std::setw(12) << "samples"
              << std::setw(14) << "estimate"
              << std::setw(14) << "error"
              << std::setw(14) << "error_bound"
              << "\n";
    std::cout << std::string(54, '-') << "\n";

    for (int n : {100, 1000, 10000, 100000, 1000000, 10000000}) {
        double pi_est = estimate_pi_monte_carlo(n);
        double error = std::abs(pi_est - std::numbers::pi);
        // Chernoff-style bound: error ~ O(1/sqrt(n))
        double bound = 3.0 / std::sqrt(static_cast<double>(n));
        std::cout << std::setw(12) << n
                  << std::setw(14) << pi_est
                  << std::setw(14) << error
                  << std::setw(14) << bound
                  << "\n";
    }

    // Multiple runs at fixed sample size to show variance
    std::cout << "\nVariance across runs (100000 samples each):\n";
    {
        double sum_est = 0;
        std::vector<double> estimates;
        for (int trial = 0; trial < 10; trial++) {
            double est = estimate_pi_monte_carlo(100000);
            estimates.push_back(est);
            sum_est += est;
        }
        double mean = sum_est / 10.0;
        double var = 0;
        for (double e : estimates) var += (e - mean) * (e - mean);
        var /= 10.0;
        std::cout << "  Mean estimate: " << mean << "\n";
        std::cout << "  Variance:      " << var << "\n";
        std::cout << "  Std dev:       " << std::sqrt(var) << "\n";
        std::cout << "  Theory (CLT):  " << std::numbers::pi * std::sqrt(std::numbers::pi / (4.0 * 100000)) << "\n";
    }

    std::cout << "\nMonte Carlo properties:\n";
    std::cout << "  - Two-sided error: estimate may be high or low\n";
    std::cout << "  - Error decreases as O(1/sqrt(n)) by CLT\n";
    std::cout << "  - To get d decimal places, need ~10^(2d) samples\n";
    std::cout << "  - Can be improved with importance sampling or variance reduction\n";

    return 0;
}
