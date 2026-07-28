// =====================================================================
// Count-Min Sketch (Cormode & Muthukrishnan, 2005)
//
// A probabilistic data structure for frequency estimation in data streams.
// Uses O(w * d) memory where w = ceil(e/eps), d = ceil(ln(1/delta)).
//
// Guarantees:
//   - Estimated frequency >= true frequency (never underestimates)
//   - Estimated frequency <= true frequency + epsilon * N
//     with probability >= 1 - delta
//
// This example tracks IP address frequencies in simulated network traffic
// and compares sketch estimates against exact counts.
// =====================================================================

#include "ral/streaming.h"
#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Count-Min Sketch: Frequency Estimation\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    // Parameters
    double eps = 0.001;    // error tolerance
    double delta = 0.01;   // failure probability
    int stream_size = 100000;

    std::cout << "Parameters: eps=" << eps << ", delta=" << delta << "\n";
    std::cout << "Stream size: " << stream_size << " packets\n\n";

    // Simulate network traffic with skewed distribution
    std::vector<std::string> traffic;
    std::vector<std::string> top_talkers = {"10.0.0.1", "10.0.0.2", "10.0.0.3"};
    std::uniform_int_distribution<int> port_dist(1, 65535);
    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);

    for (int i = 0; i < stream_size; ++i) {
        std::string src;
        if (prob_dist(rng) < 0.4) {
            src = top_talkers[static_cast<int>(prob_dist(rng) * 3)];
        } else if (prob_dist(rng) < 0.1) {
            int host = 100 + static_cast<int>(prob_dist(rng) * 50);
            src = "10.0." + std::to_string(host / 256) + "." + std::to_string(host % 256);
        } else {
            int host = 1000 + static_cast<int>(prob_dist(rng) * 10000);
            src = "192.168." + std::to_string(host / 256) + "." + std::to_string(host % 256);
        }
        traffic.push_back(src);
    }

    // Build Count-Min Sketch
    StreamingCountMinSketch cms(eps, delta, rng);

    for (const auto& ip : traffic) {
        cms.update(ip);
    }

    // Compute exact counts
    std::unordered_map<std::string, long long> exact;
    for (const auto& ip : traffic) exact[ip]++;

    // Report results
    std::cout << "Sketch dimensions: " << cms.depth() << " rows x "
              << cms.width() << " cols = " << cms.depth() * cms.width()
              << " cells\n";
    std::cout << "Exact storage would need: " << exact.size() << " entries\n\n";

    std::cout << "--- Top Talker Accuracy ---\n";
    std::cout << std::setw(15) << "IP Address"
              << std::setw(12) << "Exact"
              << std::setw(12) << "Estimated"
              << std::setw(10) << "Error"
              << "\n";

    for (const auto& ip : top_talkers) {
        long long est = cms.estimate(ip);
        long long ex = exact[ip];
        std::cout << std::setw(15) << ip
                  << std::setw(12) << ex
                  << std::setw(12) << est
                  << std::setw(10) << (est - ex)
                  << "\n";
    }

    // Show that estimates never underestimate
    std::cout << "\n--- Verify Non-Underestimation ---\n";
    int underestimates = 0;
    for (const auto& [ip, count] : exact) {
        if (cms.estimate(ip) < count) underestimates++;
    }
    std::cout << "Items underestimated: " << underestimates << " / " << exact.size() << "\n";

    // Estimate for unseen items
    std::cout << "\n--- Unseen Item Estimate ---\n";
    std::cout << "Estimate for '172.16.0.1' (never in stream): "
              << cms.estimate("172.16.0.1") << "\n";
    std::cout << "Estimate for '0.0.0.0' (never in stream): "
              << cms.estimate("0.0.0.0") << "\n";

    std::cout << "\nCount-Min Sketch provides space-efficient frequency estimation\n";
    std::cout << "with provable error bounds: est <= true + eps*N w.p. >= 1-delta.\n";

    return 0;
}
