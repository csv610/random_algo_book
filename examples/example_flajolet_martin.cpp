// =====================================================================
// Flajolet-Martin Algorithm (1984)
//
// Estimates the number of distinct elements in a data stream using
// O(p * log m) bits where p = number of registers, m = universe size.
//
// Key idea: hash each element to {0,1}^64, examine the position of the
// least significant 1-bit. The maximum position seen across all elements
// gives an estimate of log2(distinct_count).
//
// Uses group-wise averaging for variance reduction and returns the
// median of group estimates. Expected relative error: O(1/sqrt(p)).
//
// This example counts distinct IP addresses in simulated network traffic.
// =====================================================================

#include "ral/streaming.h"
#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <unordered_set>
#include <cmath>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Flajolet-Martin: Distinct Element Counting\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    int num_registers = 64;
    int stream_size = 100000;

    std::cout << "Registers: " << num_registers << "\n";
    std::cout << "Stream size: " << stream_size << " elements\n\n";

    // Generate a stream with a known number of distinct elements
    std::vector<std::string> stream;
    int num_distinct = 500;
    std::uniform_int_distribution<int> id_dist(0, num_distinct - 1);
    std::uniform_real_distribution<double> noise_dist(0.0, 1.0);

    // Create a pool of distinct elements
    std::vector<std::string> pool;
    for (int i = 0; i < num_distinct; ++i) {
        pool.push_back("host_" + std::to_string(i));
    }

    // Zipf-like distribution: some elements appear much more often
    for (int i = 0; i < stream_size; ++i) {
        // Power-law sampling: lower indices more frequent
        double u = noise_dist(rng);
        int idx = static_cast<int>(num_distinct * std::pow(u, 0.5));
        idx = std::min(idx, num_distinct - 1);
        stream.push_back(pool[idx]);
    }

    // Run Flajolet-Martin
    FlajoletMartin fm(num_registers, rng);

    for (const auto& item : stream) {
        fm.update(item);
    }

    // Compute exact distinct count
    std::unordered_set<std::string> exact_distinct(stream.begin(), stream.end());

    double estimated = fm.estimate();
    double exact = static_cast<double>(exact_distinct.size());
    double rel_error = 100.0 * std::abs(estimated - exact) / exact;

    std::cout << "--- Results ---\n";
    std::cout << "Stream length:        " << fm.stream_count() << "\n";
    std::cout << "Estimated distinct:   " << std::fixed << std::setprecision(0)
              << estimated << "\n";
    std::cout << "Exact distinct:       " << exact_distinct.size() << "\n";
    std::cout << "Relative error:       " << std::setprecision(2)
              << rel_error << "%\n";

    // Show the power of the algorithm: memory vs exact set
    std::cout << "\n--- Memory Comparison ---\n";
    std::cout << "Flajolet-Martin registers: " << num_registers
              << " x 64-bit = " << num_registers * 8 << " bytes\n";
    std::cout << "Exact set would need: ~" << exact_distinct.size() * 50
              << " bytes (string storage)\n";
    std::cout << "Compression ratio: ~"
              << std::setprecision(1)
              << (double)(exact_distinct.size() * 50) / (num_registers * 8)
              << "x\n";

    // Run multiple trials to show consistency
    std::cout << "\n--- Consistency Over 10 Trials ---\n";
    std::cout << std::setw(8) << "Trial"
              << std::setw(14) << "Estimate"
              << std::setw(12) << "Error%"
              << "\n";

    for (int trial = 0; trial < 10; ++trial) {
        std::mt19937 trial_rng(1000 + trial);
        FlajoletMartin trial_fm(num_registers, trial_rng);
        for (const auto& item : stream) {
            trial_fm.update(item);
        }
        double est = trial_fm.estimate();
        double err = 100.0 * std::abs(est - exact) / exact;
        std::cout << std::setw(8) << trial + 1
                  << std::setw(14) << std::fixed << std::setprecision(0) << est
                  << std::setw(12) << std::setprecision(2) << err << "%"
                  << "\n";
    }

    std::cout << "\nFlajolet-Martin provides a sub-linear space estimate of\n";
    std::cout << "distinct elements with O(1/sqrt(p)) relative error.\n";

    return 0;
}
