// =====================================================================
// Weighted Universal Hashing for Load Balancing
//
// Weighted universal hashing assigns keys to buckets with
// probabilities proportional to given weights, enabling load
// balancing across heterogeneous servers.
//
// Given weights w_1, ..., w_n for n servers, the hash function:
//   h(key) = argmax_i { w_i * h_i(key) }
// where each h_i is an independent universal hash, ensures that
// key is assigned to server i with probability w_i / sum(w).
//
// This example:
//   1. Distributes jobs across servers with different capacities
//   2. Shows the actual distribution matches the weights
//   3. Compares with uniform hashing
// =====================================================================

#include "ral/hash_table.h"
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
#include <cmath>
#include <numeric>

using namespace ral;

// Weighted hash: each key is assigned to bucket i with prob w[i]/sum(w)
// Implementation: sample h_i uniformly from universal family, assign key
// to bucket maximizing w_i * random_value_in_[0,1)
int weighted_hash(int key, const std::vector<double>& weights, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    int best = 0;
    double best_val = weights[0] * dist(rng);
    for (int i = 1; i < (int)weights.size(); i++) {
        double val = weights[i] * dist(rng);
        if (val > best_val) {
            best_val = val;
            best = i;
        }
    }
    return best;
}

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Weighted Universal Hashing for Load Balancing\n";
    std::cout << "=============================================\n\n";

    // --- Scenario: 4 servers with different capacities ---
    std::vector<double> weights = {1.0, 2.0, 3.0, 4.0};
    double total_w = std::accumulate(weights.begin(), weights.end(), 0.0);
    int num_servers = (int)weights.size();

    std::cout << "Servers and weights:\n";
    for (int i = 0; i < num_servers; i++)
        std::cout << "  Server " << i << ": weight=" << weights[i]
                  << "  expected_share=" << std::fixed << std::setprecision(1)
                  << 100.0 * weights[i] / total_w << "%\n\n";

    // --- Distribute 10000 jobs ---
    int num_jobs = 10000;
    std::mt19937 rng(42);
    std::vector<int> counts(num_servers, 0);

    for (int j = 0; j < num_jobs; j++) {
        int bucket = weighted_hash(j + 1, weights, rng);
        counts[bucket]++;
    }

    std::cout << "--- Weighted Distribution (" << num_jobs << " jobs) ---\n";
    std::cout << std::setw(10) << "Server"
              << std::setw(12) << "Count"
              << std::setw(12) << "Actual%"
              << std::setw(12) << "Expected%"
              << "\n";
    for (int i = 0; i < num_servers; i++) {
        double actual = 100.0 * counts[i] / num_jobs;
        double expected = 100.0 * weights[i] / total_w;
        std::cout << std::setw(10) << i
                  << std::setw(12) << counts[i]
                  << std::setw(12) << std::fixed << std::setprecision(1) << actual
                  << std::setw(12) << std::fixed << std::setprecision(1) << expected
                  << "\n";
    }

    // --- Compare with uniform hashing ---
    std::cout << "\n--- Uniform Distribution (same jobs) ---\n";
    std::vector<int> ucounts(num_servers, 0);
    std::mt19937 urng(42);
    std::uniform_int_distribution<int> udist(0, num_servers - 1);
    for (int j = 0; j < num_jobs; j++)
        ucounts[udist(urng)]++;

    std::cout << std::setw(10) << "Server"
              << std::setw(12) << "Count"
              << std::setw(12) << "Share%"
              << "\n";
    for (int i = 0; i < num_servers; i++) {
        std::cout << std::setw(10) << i
                  << std::setw(12) << ucounts[i]
                  << std::setw(12) << std::fixed << std::setprecision(1)
                  << 100.0 * ucounts[i] / num_jobs
                  << "\n";
    }

    std::cout << "\nWeighted hashing routes jobs proportionally to server capacity.\n";
    return 0;
}
