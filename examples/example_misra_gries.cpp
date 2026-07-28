// =====================================================================
// Misra-Gries Heavy Hitter Detection (1982)
//
// Finds all items with frequency > n/k in O(n) time and O(k) space.
//
// Guarantees:
//   - All items with frequency > n/k are reported (no false negatives)
//   - No item with frequency < n/k - n/k^2 is reported
//   - Estimated counts are within n/k of true counts
//
// This example detects heavy-hitter IP addresses in simulated network
// traffic, identifying sources that send a disproportionate number of
// packets.
// =====================================================================

#include "ral/streaming.h"
#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <unordered_map>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Misra-Gries: Heavy Hitter Detection\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    int k = 10;  // number of counters
    int stream_size = 50000;

    std::cout << "Parameters: k=" << k << " counters, stream size=" << stream_size << "\n";
    std::cout << "Heavy hitter threshold: n/k = " << stream_size / k << " packets\n\n";

    // Generate traffic with known heavy hitters
    std::vector<std::string> traffic;
    std::uniform_real_distribution<double> prob(0.0, 1.0);

    // 3 known heavy hitters
    std::vector<std::pair<std::string, double>> sources = {
        {"server_A", 0.15},   // 15% of traffic
        {"server_B", 0.12},   // 12% of traffic
        {"server_C", 0.10},   // 10% of traffic
        {"normal_1", 0.03},
        {"normal_2", 0.03},
        {"normal_3", 0.02},
        {"normal_4", 0.02},
        {"normal_5", 0.01}
    };

    for (int i = 0; i < stream_size; ++i) {
        double r = prob(rng);
        double cumulative = 0.0;
        for (const auto& [name, freq] : sources) {
            cumulative += freq;
            if (r < cumulative) {
                traffic.push_back(name);
                break;
            }
        }
        if (static_cast<int>(traffic.size()) <= i) {
            // long tail: random background traffic
            traffic.push_back("bg_" + std::to_string(static_cast<int>(prob(rng) * 1000)));
        }
    }

    // Run Misra-Gries
    MisraGries mg(k);
    for (const auto& src : traffic) {
        mg.update(src);
    }

    // Compute exact counts
    std::unordered_map<std::string, long long> exact;
    for (const auto& src : traffic) exact[src]++;

    // Get candidates
    auto candidates = mg.candidates();
    long long threshold = stream_size / k;

    std::cout << "--- Candidates Found ---\n";
    std::cout << std::setw(12) << "Source"
              << std::setw(12) << "Estimated"
              << std::setw(12) << "Exact"
              << std::setw(10) << "Status"
              << "\n";

    for (const auto& c : candidates) {
        long long ex = exact[c.item];
        bool is_heavy = ex > threshold;
        std::cout << std::setw(12) << c.item
                  << std::setw(12) << c.estimated_count
                  << std::setw(12) << ex
                  << std::setw(10) << (is_heavy ? "HEAVY" : "light")
                  << "\n";
    }

    // Verify: all true heavy hitters are candidates
    std::cout << "\n--- Verification ---\n";
    int true_heavy_found = 0;
    int true_heavy_total = 0;
    for (const auto& [src, count] : exact) {
        if (count > threshold) {
            true_heavy_total++;
            if (mg.estimate(src) > 0) true_heavy_found++;
        }
    }
    std::cout << "True heavy hitters found: " << true_heavy_found
              << " / " << true_heavy_total << "\n";
    std::cout << "Total candidates reported: " << candidates.size() << "\n";
    std::cout << "Space used: O(k) = " << k << " counters\n";

    // Estimate accuracy
    std::cout << "\n--- Count Estimation Error ---\n";
    long long total_error = 0;
    for (const auto& c : candidates) {
        long long ex = exact[c.item];
        total_error += std::abs(c.estimated_count - ex);
    }
    std::cout << "Average absolute error per candidate: "
              << (candidates.empty() ? 0 : total_error / static_cast<long long>(candidates.size()))
              << "\n";

    std::cout << "\nMisra-Gries identifies heavy hitters in a single pass\n";
    std::cout << "with O(k) space, guaranteeing no false negatives above n/k.\n";

    return 0;
}
