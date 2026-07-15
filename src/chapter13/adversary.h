#pragma once
// Chapter 13: Adversary Models for Online Algorithms
// Oblivious vs. adaptive adversaries and their impact on competitive ratios.

#include <iostream>
#include <vector>
#include <functional>
#include <random>
#include <algorithm>
#include <numeric>
#include <print>
#include <format>
#include <ranges>
#include <cmath>
#include <limits>

#include "../chapter10/random_utils.h"

namespace randalgo {

// ============================================================
// ObliviousAdversary
// Generates an entire worst-case request sequence before seeing
// the algorithm's responses. Models the strongest offline adversary.
// ============================================================

class ObliviousAdversary {
public:
    // Generate a worst-case sequence for LRU/FIFO with k-page cache.
    // The classic adversarial sequence: cyclic permutation of k+1 pages.
    // This forces k+1 pages through a cache of size k, ensuring
    // a miss on every other request.
    static std::vector<int> worst_case_lru_fifo(int k, int length) {
        std::vector<int> sequence;
        int alphabet_size = k + 1;
        for (int i = 0; i < length; ++i)
            sequence.push_back(i % alphabet_size + 1);
        return sequence;
    }

    // Generate a worst-case sequence for a specific deterministic algorithm.
    // Simulates the algorithm online, then picks the request that causes
    // the most harm (works because the adversary knows the algorithm).
    template<typename AlgFunc>
    static std::vector<int> worst_case_for_deterministic(
        AlgFunc algorithm, int cache_size, int length)
    {
        // Build the sequence greedily: at each step, try each possible page
        // and pick the one that maximizes the algorithm's cost.
        std::vector<int> sequence;
        std::vector<int> current_requests;

        // We simulate the algorithm forward for each candidate
        for (int step = 0; step < length; ++step) {
            int best_page = 1;
            int worst_cost = -1;

            for (int candidate = 1; candidate <= cache_size + 1; ++candidate) {
                auto test_requests = current_requests;
                test_requests.push_back(candidate);
                int cost = algorithm(cache_size, test_requests, false);
                if (cost > worst_cost) {
                    worst_cost = cost;
                    best_page = candidate;
                }
            }
            sequence.push_back(best_page);
            current_requests.push_back(best_page);
        }
        return sequence;
    }

    // Generate a cyclic adversary sequence with parameterized period
    static std::vector<int> cyclic_sequence(int period, int length) {
        std::vector<int> seq;
        for (int i = 0; i < length; ++i)
            seq.push_back(i % period + 1);
        return seq;
    }

    // Generate a "interleaved" adversary: alternates between two groups
    // to maximize misses for LRU/FIFO
    static std::vector<int> interleaved_adversary(int k, int length) {
        std::vector<int> seq;
        // Group A: {1,...,k}, Group B: {k+1}
        // Pattern: 1, k+1, 2, k+1, 3, k+1, ... forces evictions
        for (int i = 0; i < length; ++i) {
            if (i % 2 == 0)
                seq.push_back((i / 2) % k + 1);
            else
                seq.push_back(k + 1);
        }
        return seq;
    }
};

// ============================================================
// AdaptiveOnlineAdversary
// Generates requests one at a time, observing the algorithm's
// current state (cache contents or server positions).
// Models a stronger adversary that reacts to the algorithm.
// ============================================================

class AdaptiveOnlineAdversary {
public:
    // For paging: at each step, observe the cache contents and
    // pick a request NOT in the cache (causing a miss).
    // If all pages 1..k+1 are in cache, pick any page not in cache.
    static std::vector<int> adaptive_paging_adversary(
        std::function<std::vector<int>(const std::vector<int>&)> get_cache_state,
        int /*cache_size*/,
        int alphabet_size,
        int length)
    {
        std::vector<int> sequence;
        std::vector<int> history;

        for (int step = 0; step < length; ++step) {
            auto cache = get_cache_state(history);
            std::unordered_set<int> in_cache(cache.begin(), cache.end());

            // Find a page not in cache
            int chosen = -1;
            for (int p = 1; p <= alphabet_size; ++p) {
                if (!in_cache.contains(p)) {
                    chosen = p;
                    break;
                }
            }

            // If all alphabet pages are in cache (shouldn't happen if
            // alphabet_size > cache_size), pick page 1
            if (chosen == -1) chosen = 1;

            sequence.push_back(chosen);
            history.push_back(chosen);
        }
        return sequence;
    }

    // For k-server: observe server positions and pick request far
    // from all servers to maximize movement cost
    static std::vector<int> adaptive_server_adversary(
        std::function<std::vector<int>(const std::vector<int>&)> get_server_positions,
        const std::vector<std::vector<int>>& dist,
        int num_positions,
        const std::vector<int>& initial_positions,
        int length)
    {
        std::vector<int> sequence;
        std::vector<int> server_pos = initial_positions;

        for (int step = 0; step < length; ++step) {
            server_pos = get_server_positions(sequence);

            // Pick the position farthest from all servers
            int best_pos = 0;
            int best_min_dist = -1;
            for (int p = 0; p < num_positions; ++p) {
                int min_d = std::numeric_limits<int>::max();
                for (int s : server_pos)
                    min_d = std::min(min_d, dist[s][p]);
                if (min_d > best_min_dist) {
                    best_min_dist = min_d;
                    best_pos = p;
                }
            }
            sequence.push_back(best_pos);
        }
        return sequence;
    }
};

// ============================================================
// Adversarial Test: pit adversary against algorithm, measure ratio
// ============================================================

struct AdversaryTestResult {
    int algorithm_cost;
    int optimal_cost;
    double competitive_ratio;
    std::vector<int> requests;
};

// For paging
AdversaryTestResult adversarial_test(
    std::function<int(int, const std::vector<int>&, bool)> algorithm,
    std::function<int(int, const std::vector<int>&, bool)> optimal,
    int cache_size,
    const std::vector<int>& requests)
{
    int alg_cost = algorithm(cache_size, requests, false);
    int opt_cost = optimal(cache_size, requests, false);
    double ratio = (opt_cost > 0)
        ? static_cast<double>(alg_cost) / static_cast<double>(opt_cost)
        : 1.0;
    return {alg_cost, opt_cost, ratio, requests};
}

// ============================================================
// Demonstration
// ============================================================

void demonstrate_adversary() {
    std::println("=== Adversary Models (Chapter 13) ===\n");

    // ---- Oblivious Adversary ----
    std::println("--- Oblivious Adversary ---");
    std::println("The adversary fixes the entire sequence before the algorithm runs.\n");

    {
        int k = 3;
        auto worst_seq = ObliviousAdversary::worst_case_lru_fifo(k, 12);
        std::println("  Worst-case sequence for LRU/FIFO (k={}, length=12):", k);
        std::print("    ");
        for (int r : worst_seq) std::print("{} ", r);
        std::println("");

        auto result = adversarial_test(deterministic_lru, optimal_offline, k, worst_seq);
        std::println("  LRU cost:    {}  OPT: {}  ratio: {:.2f}",
                     result.algorithm_cost, result.optimal_cost, result.competitive_ratio);

        auto result2 = adversarial_test(deterministic_fifo, optimal_offline, k, worst_seq);
        std::println("  FIFO cost:   {}  OPT: {}  ratio: {:.2f}",
                     result2.algorithm_cost, result2.optimal_cost, result2.competitive_ratio);

        auto result3 = adversarial_test(random_paging, optimal_offline, k, worst_seq);
        std::println("  Random cost: {}  OPT: {}  ratio: {:.2f} (one trial)",
                     result3.algorithm_cost, result3.optimal_cost, result3.competitive_ratio);
        std::println("");
    }

    // ---- Adaptive Adversary ----
    std::println("--- Adaptive Online Adversary ---");
    std::println("The adversary observes the algorithm's state at each step.\n");

    {
        int k = 3;
        int alphabet = 5;
        // The adaptive adversary for LRU
        // We need to simulate LRU to get cache states, then feed
        // adversarial requests. We do this round-trip.

        // Lambda: given history, run LRU and return cache snapshot
        auto lru_cache_state = [](const std::vector<int>& history) -> std::vector<int> {
            PageCache cache(3);
            for (int p : history) {
                if (cache.contains(p)) cache.touch(p);
                else cache.insert(p);
            }
            return cache.snapshot();
        };

        auto adseq = AdaptiveOnlineAdversary::adaptive_paging_adversary(
            lru_cache_state, k, alphabet, 20);

        std::print("    Adaptive sequence: ");
        for (int r : adseq) std::print("{} ", r);
        std::println("");

        auto result = adversarial_test(deterministic_lru, optimal_offline, k, adseq);
        std::println("  LRU cost:    {}  OPT: {}  ratio: {:.2f}",
                     result.algorithm_cost, result.optimal_cost, result.competitive_ratio);

        // Randomized algorithm on the same adaptive sequence
        // (adversary adapts to deterministic LRU, not to random)
        auto result_rand = adversarial_test(random_paging, optimal_offline, k, adseq);
        std::println("  Random cost: {}  OPT: {}  ratio: {:.2f} (on LRU-adapted sequence)",
                     result_rand.algorithm_cost, result_rand.optimal_cost,
                     result_rand.competitive_ratio);
        std::println("");
    }

    // ---- Comparison: Oblivious vs Adaptive ----
    std::println("--- Oblivious vs Adaptive Adversary ---\n");
    {
        int k = 3;
        constexpr int trials = 20;

        // Oblivious: fixed worst-case sequence
        auto oblivious_seq = ObliviousAdversary::worst_case_lru_fifo(k, 100);

        double oblivious_ratio_lru = 0, oblivious_ratio_rand = 0;
        for (int t = 0; t < trials; ++t) {
            auto r1 = adversarial_test(deterministic_lru, optimal_offline, k, oblivious_seq);
            auto r2 = adversarial_test(random_paging, optimal_offline, k, oblivious_seq);
            oblivious_ratio_lru += r1.competitive_ratio;
            oblivious_ratio_rand += r2.competitive_ratio;
        }

        std::println("  Oblivious adversary (fixed sequence, length 100):");
        std::println("    LRU avg ratio:    {:.2f}", oblivious_ratio_lru / trials);
        std::println("    Random avg ratio: {:.2f}", oblivious_ratio_rand / trials);

        // Adaptive: generates sequence that's bad for LRU specifically
        auto lru_cache_state = [](const std::vector<int>& history) -> std::vector<int> {
            PageCache cache(3);
            for (int p : history) {
                if (cache.contains(p)) cache.touch(p);
                else cache.insert(p);
            }
            return cache.snapshot();
        };

        auto adaptive_seq = AdaptiveOnlineAdversary::adaptive_paging_adversary(
            lru_cache_state, k, 5, 100);

        auto adaptive_lru = adversarial_test(deterministic_lru, optimal_offline, k, adaptive_seq);
        auto adaptive_rand = adversarial_test(random_paging, optimal_offline, k, adaptive_seq);

        std::println("\n  Adaptive adversary (adapted to LRU, length 100):");
        std::println("    LRU ratio:    {:.2f}", adaptive_lru.competitive_ratio);
        std::println("    Random ratio: {:.2f}", adaptive_rand.competitive_ratio);
        std::println("");
    }

    // ---- Randomization Helps Against Oblivious Adversaries ----
    std::println("--- Why Randomization Helps ---\n");
    {
        int k = 4;
        std::println("  For k={}, deterministic algorithms can be forced to k-competitive.\n", k);

        // Show that random achieves H_k ~ ln(k) instead
        auto worst_seq = ObliviousAdversary::worst_case_lru_fifo(k, 200);
        constexpr int trials = 50;

        auto det_result = adversarial_test(deterministic_lru, optimal_offline, k, worst_seq);
        double rand_avg_ratio = 0;
        for (int t = 0; t < trials; ++t) {
            auto r = adversarial_test(random_paging, optimal_offline, k, worst_seq);
            rand_avg_ratio += r.competitive_ratio;
        }

        double hk = 0.0;
        for (int i = 1; i <= k; ++i) hk += 1.0 / i;

        std::println("  k={}, H_k = {:.4f} (~ ln(k) = {:.4f})", k, hk, std::log(k));
        std::println("  Deterministic LRU ratio:  {:.2f} (theory: {})", det_result.competitive_ratio, k);
        std::println("  Randomized avg ratio:     {:.2f} (theory: H_k = {:.2f})",
                     rand_avg_ratio / trials, hk);
        std::println("  Speedup from randomization: {:.2f}x",
                     det_result.competitive_ratio / (rand_avg_ratio / trials));
        std::println("");
    }

    // ---- Different Adversarial Strategies ----
    std::println("--- Different Adversarial Strategies ---\n");
    {
        int k = 3;
        constexpr int length = 30;

        // Strategy 1: Cyclic (k+1 pages)
        auto seq1 = ObliviousAdversary::worst_case_lru_fifo(k, length);

        // Strategy 2: Interleaved
        auto seq2 = ObliviousAdversary::interleaved_adversary(k, length);

        // Strategy 3: Uniform random
        std::vector<int> seq3(length);
        for (auto& s : seq3) s = rng().rand_int(1, k + 1);

        // Strategy 4: Zipf-like (heavy hitters)
        std::vector<int> seq4(length);
        for (auto& s : seq4) {
            // Power-law: page i chosen with probability ~ 1/i
            double r = rng().rand_double();
            double cumsum = 0;
            s = 1;
            for (int p = 1; p <= k + 2; ++p) {
                cumsum += 1.0 / p;
                if (r * (1.0 + std::log(k + 2.0)) <= cumsum) { s = p; break; }
            }
        }

        struct StratResult { std::string name; double lru_ratio; double rand_ratio; };
        std::vector<StratResult> results;

        for (auto [name, seq] : {
            std::tuple{"Cyclic", seq1},
            std::tuple{"Interleaved", seq2},
            std::tuple{"Uniform", seq3},
            std::tuple{"Zipf-like", seq4}
        }) {
            auto lr = adversarial_test(deterministic_lru, optimal_offline, k, seq);
            double rr_sum = 0;
            for (int t = 0; t < 20; ++t)
                rr_sum += adversarial_test(random_paging, optimal_offline, k, seq).competitive_ratio;
            results.push_back({name, lr.competitive_ratio, rr_sum / 20});
        }

        std::println("  {:>14s}  {:>12s}  {:>12s}", "Strategy", "LRU ratio", "Random ratio");
        std::println("  {:>14s}  {:>12s}  {:>12s}", "--------", "---------", "-----------");
        for (auto& [name, lr, rr] : results)
            std::println("  {:>14s}  {:>12.2f}  {:>12.2f}", name, lr, rr);
        std::println("");
    }

    std::println("--- Key Insights ---");
    std::println("  1. Oblivious adversary: worst-case over ALL input sequences");
    std::println("  2. Adaptive adversary: worst-case over sequences that adapt to algorithm");
    std::println("  3. Randomization buys immunity against oblivious adversaries");
    std::println("  4. Against adaptive adversaries, randomization helps less");
    std::println("  5. The competitive ratio measures worst-case online performance\n");
}

} // namespace randalgo
