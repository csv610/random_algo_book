#pragma once

#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>

namespace chapter4 {

// Simulate random routing of n packets to n memory modules.
// Each packet independently selects a destination uniformly at random.
// Returns the maximum queue length (congestion).
int simulate_routing(int n, std::mt19937& rng) {
    std::vector<int> queues(n, 0);
    std::uniform_int_distribution<int> dist(0, n - 1);
    for (int i = 0; i < n; i++) {
        queues[dist(rng)]++;
    }
    return *std::max_element(queues.begin(), queues.end());
}

// Simulate routing over multiple rounds.
// In each round, each processor sends a packet to a random destination.
// After each round, delivered packets are removed.
// Returns the number of rounds needed until all packets are delivered.
int simulate_routing_rounds(int n, std::mt19937& rng) {
    std::vector<int> remaining(n, 1); // each processor has 1 packet
    int rounds = 0;
    std::uniform_int_distribution<int> dist(0, n - 1);

    while (true) {
        bool all_done = true;
        for (int i = 0; i < n; i++) {
            if (remaining[i] > 0) all_done = false;
        }
        if (all_done) break;

        rounds++;
        // Count destinations chosen this round
        std::vector<int> dest_count(n, 0);
        for (int i = 0; i < n; i++) {
            if (remaining[i] > 0) {
                int dest = dist(rng);
                dest_count[dest]++;
            }
        }

        // Each module serves one packet per round (FIFO)
        // Packets that share a destination compete; only one survives per round
        for (int i = 0; i < n; i++) {
            if (remaining[i] > 0) {
                remaining[i] = 0; // send this packet
            }
        }
        // Actually, in a single round, multiple packets can go to the same
        // destination, but only one is delivered. The rest remain.
        // Re-model: each processor independently picks a destination;
        // if multiple target the same module, only 1 is delivered.
        // This requires a more careful simulation.
        // Simplified model: all packets sent simultaneously,
        // at most 1 delivered per module per round.
        std::vector<bool> delivered(n, false);
        std::vector<int> dests;
        for (int i = 0; i < n; i++) {
            // Restore: we need to track which are still pending
            // Restart the round with a cleaner approach
        }
        break; // use simpler model below
    }

    // Simpler model: packets are routed simultaneously.
    // Each module delivers at most 1 packet per round.
    // Remaining packets stay in queue.
    std::vector<int> pending(n, 1);
    rounds = 0;

    while (true) {
        bool all_done = true;
        for (int i = 0; i < n; i++) {
            if (pending[i] > 0) { all_done = false; break; }
        }
        if (all_done) break;

        rounds++;
        std::vector<int> dests(n);
        std::vector<int> serve_count(n, 0);

        for (int i = 0; i < n; i++) {
            dests[i] = dist(rng);
        }

        // Each module serves at most 1 packet
        std::vector<bool> served_module(n, false);
        for (int i = 0; i < n; i++) {
            if (pending[i] > 0 && !served_module[dests[i]]) {
                pending[i] = 0;
                served_module[dests[i]] = true;
            }
        }

        if (rounds > 100 * n) break; // safety
    }

    return rounds;
}

// Compute the theoretical Chernoff-based upper bound on max congestion
// Pr[max queue >= t] <= n * exp(-mu*(t*ln(t/mu) - (t - mu)))
// for mu = 1 (each module gets expected 1 packet)
double theoretical_congestion_bound(int n, double t) {
    if (t <= 1.0) return 1.0;
    double mu = 1.0;
    double log_ratio = std::log(t / mu);
    double exponent = -mu * (t * log_ratio - (t - mu));
    return n * std::exp(exponent);
}

// Find the smallest t such that the Chernoff bound gives Pr[max >= t] < 1/n^c
int theoretical_max_congestion(int n, double c) {
    double target = std::pow(n, -c);
    for (int t = 1; t <= n; t++) {
        if (theoretical_congestion_bound(n, static_cast<double>(t)) < target) {
            return t;
        }
    }
    return n;
}

void demonstrate_routing() {
    std::mt19937 rng(42);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Routing in a Parallel Computer\n";
    std::cout << std::string(58, '-') << "\n\n";

    // Experiment 1: Max congestion vs n
    std::cout << "Experiment 1: Maximum Congestion vs. Number of Processors\n\n";
    std::cout << std::setw(8) << "n"
              << std::setw(12) << "Avg Max Q"
              << std::setw(12) << "Theory t*"
              << std::setw(14) << "ln(n)/lnln(n)"
              << std::setw(14) << "Bound(Theory)"
              << "\n";
    std::cout << std::string(60, '-') << "\n";

    std::vector<int> ns = {50, 100, 200, 500, 1000, 2000};
    int sim_trials = 200;

    for (int n : ns) {
        long long total_max_q = 0;
        for (int trial = 0; trial < sim_trials; trial++) {
            total_max_q += simulate_routing(n, rng);
        }
        double avg_max = static_cast<double>(total_max_q) / sim_trials;
        int theory_t = theoretical_max_congestion(n, 2.0);
        double ln_n = std::log(static_cast<double>(n));
        double ln_ln_n = std::log(ln_n);
        double approx = ln_n / ln_ln_n;
        double bound_at_approx = theoretical_congestion_bound(n, approx);

        std::cout << std::setw(8) << n
                  << std::setw(12) << avg_max
                  << std::setw(12) << theory_t
                  << std::setw(14) << approx
                  << std::setw(14) << bound_at_approx
                  << "\n";
    }

    // Experiment 2: Probability distribution of max congestion for n=500
    std::cout << "\n\nExperiment 2: Distribution of Max Queue Length (n=500, 1000 trials)\n\n";
    const int n2 = 500;
    const int trials2 = 1000;
    std::vector<int> hist(15, 0);

    for (int t = 0; t < trials2; t++) {
        int mq = simulate_routing(n2, rng);
        if (mq < 15) hist[mq]++;
    }

    std::cout << std::setw(10) << "Max Queue"
              << std::setw(10) << "Count"
              << std::setw(12) << "Frequency"
              << "\n";
    std::cout << std::string(32, '-') << "\n";

    for (int i = 1; i < 15; i++) {
        if (hist[i] > 0) {
            std::cout << std::setw(10) << i
                      << std::setw(10) << hist[i]
                      << std::setw(12) << static_cast<double>(hist[i]) / trials2
                      << "\n";
        }
    }

    // Experiment 3: Routing rounds
    std::cout << "\n\nExperiment 3: Number of Rounds to Deliver All Packets\n\n";
    std::cout << std::setw(8) << "n"
              << std::setw(12) << "Avg Rounds"
              << std::setw(14) << "ln(n)"
              << "\n";
    std::cout << std::string(34, '-') << "\n";

    std::vector<int> ns_round = {10, 20, 50, 100, 200};
    for (int n : ns_round) {
        long long total_rounds = 0;
        int round_trials = 500;
        for (int t = 0; t < round_trials; t++) {
            total_rounds += simulate_routing_rounds(n, rng);
        }
        double avg = static_cast<double>(total_rounds) / round_trials;
        std::cout << std::setw(8) << n
                  << std::setw(12) << avg
                  << std::setw(14) << std::log(static_cast<double>(n))
                  << "\n";
    }

    std::cout << "\nKey observations:\n";
    std::cout << "  - Max congestion grows as O(ln n / ln ln n), much slower than n\n";
    std::cout << "  - The Chernoff bound accurately predicts the concentration\n";
    std::cout << "  - Most processors see only 0 or 1 packets; few see many\n";
    std::cout << "  - Random routing achieves near-optimal load balancing\n";
}

} // namespace chapter4
