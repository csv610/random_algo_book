/*
 * Randomized Packet Routing on Networks
 *
 * Simulates routing n packets from n processors to n memory modules,
 * where each packet independently selects a random destination.
 *
 * Key results:
 *   - The maximum queue length (concentration at any module) is
 *     O(ln n / ln ln n) with high probability.
 *   - This follows from Chernoff bounds applied to binomial
 *     random variables (each module receives ~Binomial(n, 1/n) packets).
 *   - Random routing achieves near-optimal load balancing.
 *
 * Reference: "Routing, Broadcasting, and Sorting" - Leighton, Maggs, Rao
 */

#include "ral/routing.h"
#include <iostream>
#include <iomanip>
#include <random>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Randomized Packet Routing\n";
    std::cout << "========================================================\n\n";

    std::mt19937 rng(42);

    // Experiment 1: Max congestion for various n
    std::cout << "Experiment 1: Maximum congestion vs network size\n";
    std::cout << std::setw(8) << "n"
              << std::setw(14) << "Avg Max Queue"
              << std::setw(14) << "Theory t*"
              << std::setw(14) << "ln(n)/lnln(n)"
              << "\n";
    std::cout << std::string(50, '-') << "\n";

    for (int n : {50, 100, 200, 500, 1000}) {
        long long total_max_q = 0;
        int trials = 500;
        for (int t = 0; t < trials; t++)
            total_max_q += simulate_routing(n, rng);

        double avg_max = static_cast<double>(total_max_q) / trials;
        int theory_t = theoretical_max_congestion(n, 2.0);
        double ln_n = std::log(static_cast<double>(n));
        double ln_ln_n = std::log(ln_n);

        std::cout << std::setw(8) << n
                  << std::setw(14) << std::fixed << std::setprecision(2) << avg_max
                  << std::setw(14) << theory_t
                  << std::setw(14) << std::setprecision(2) << ln_n / ln_ln_n
                  << "\n";
    }

    // Experiment 2: Distribution of max queue for n=500
    std::cout << "\nExperiment 2: Queue distribution (n=500, 1000 trials)\n";
    std::vector<int> hist(10, 0);
    for (int t = 0; t < 1000; t++) {
        int mq = simulate_routing(500, rng);
        if (mq < 10) hist[mq]++;
    }
    for (int i = 1; i < 10; i++) {
        if (hist[i] > 0) {
            std::cout << "  Queue=" << i << ": " << hist[i] << " trials ("
                      << std::fixed << std::setprecision(3)
                      << 100.0 * hist[i] / 1000 << "%)\n";
        }
    }

    // Experiment 3: Routing rounds
    std::cout << "\nExperiment 3: Rounds to deliver all packets\n";
    for (int n : {10, 20, 50, 100}) {
        long long total_rounds = 0;
        int trials = 200;
        for (int t = 0; t < trials; t++)
            total_rounds += simulate_routing_rounds(n, rng);
        double avg = static_cast<double>(total_rounds) / trials;
        std::cout << "  n=" << std::setw(4) << n
                  << " avg rounds=" << std::setw(6) << std::fixed << std::setprecision(2) << avg
                  << "  (ln n = " << std::setprecision(2) << std::log(double(n)) << ")\n";
    }

    std::cout << "\nSummary: Max congestion grows as O(ln n / ln ln n)\n";

    return 0;
}
