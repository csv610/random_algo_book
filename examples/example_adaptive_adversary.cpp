// example_adaptive_adversary.cpp
// Demonstrates the adaptive online adversary model for paging algorithms.
//
// Unlike an oblivious adversary, an adaptive adversary observes the
// algorithm's current state (cache contents) at each step and chooses
// the next request to cause maximum harm.
//
// Key insight: an adaptive adversary can always pick a page NOT in the
// algorithm's cache, guaranteeing a miss. This makes the problem harder
// for the algorithm, and randomization helps less.
//
// We compare LRU vs Random against both oblivious and adaptive adversaries.

#include "ral/paging.h"
#include "ral/adversary.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>
#include <unordered_set>

using namespace ral;

int main() {
    std::cout << "=== Adaptive Online Adversary for Paging ===\n\n";

    int k = 3;
    int alphabet = 5;
    int length = 20;

    // Define how to get LRU's cache state given a history
    auto lru_cache_state = [](const std::vector<int>& history) -> std::vector<int> {
        PageCache cache(3);
        for (int p : history) {
            if (cache.contains(p)) cache.touch(p);
            else cache.insert(p);
        }
        return cache.snapshot();
    };

    // Part 1: Generate adaptive sequence
    std::cout << "Adaptive adversary: picks page NOT in LRU's cache\n";
    std::cout << "k=" << k << ", alphabet=" << alphabet
              << ", length=" << length << "\n\n";

    auto adaptive_seq = AdaptiveOnlineAdversary::adaptive_paging_adversary(
        lru_cache_state, k, alphabet, length);

    std::cout << "  Adaptive sequence: ";
    for (int r : adaptive_seq) std::cout << r << " ";
    std::cout << "\n\n";

    // Part 2: Compare LRU vs Random on adaptive sequence
    int lru_misses = deterministic_lru(k, adaptive_seq, false);
    int rand_misses = random_paging(k, adaptive_seq, false);
    int opt_misses = optimal_offline(k, adaptive_seq, false);

    std::cout << "  Results against adaptive sequence:\n";
    std::cout << "  LRU:    " << lru_misses << " misses  ratio=" << std::setprecision(2)
              << static_cast<double>(lru_misses) / opt_misses << "\n";
    std::cout << "  Random: " << rand_misses << " misses  ratio="
              << static_cast<double>(rand_misses) / opt_misses << " (one trial)\n";
    std::cout << "  OPT:    " << opt_misses << " misses\n\n";

    // Part 3: Oblivious vs Adaptive - side by side
    std::cout << "Oblivious vs Adaptive adversary comparison:\n\n";

    int seq_len = 100;
    auto oblivious_seq = ObliviousAdversary::worst_case_lru_fifo(k, seq_len);

    // Oblivious results
    double obl_lru = static_cast<double>(deterministic_lru(k, oblivious_seq, false))
                   / optimal_offline(k, oblivious_seq, false);
    double obl_rand_sum = 0;
    for (int t = 0; t < 20; t++)
        obl_rand_sum += static_cast<double>(random_paging(k, oblivious_seq, false))
                      / optimal_offline(k, oblivious_seq, false);

    // Adaptive results
    auto adseq = AdaptiveOnlineAdversary::adaptive_paging_adversary(
        lru_cache_state, k, alphabet, seq_len);
    double adp_lru = static_cast<double>(deterministic_lru(k, adseq, false))
                   / optimal_offline(k, adseq, false);
    double adp_rand_sum = 0;
    for (int t = 0; t < 20; t++)
        adp_rand_sum += static_cast<double>(random_paging(k, adseq, false))
                      / optimal_offline(k, adseq, false);

    std::cout << std::setw(16) << "Adversary type"
              << std::setw(12) << "LRU ratio"
              << std::setw(14) << "Random ratio"
              << "\n";
    std::cout << std::string(42, '-') << "\n";
    std::cout << std::setw(16) << "Oblivious"
              << std::setw(12) << std::setprecision(2) << obl_lru
              << std::setw(14) << (obl_rand_sum / 20.0)
              << "\n";
    std::cout << std::setw(16) << "Adaptive"
              << std::setw(12) << adp_lru
              << std::setw(14) << (adp_rand_sum / 20.0)
              << "\n";

    // Part 4: Why adaptive is harder
    std::cout << "\nWhy adaptive adversaries are harder:\n";
    std::cout << "  - Adversary sees cache state and requests a page NOT in cache\n";
    std::cout << "  - This guarantees a miss for any deterministic algorithm\n";
    std::cout << "  - Randomization helps less because adversary adapts to state\n";
    std::cout << "  - For adaptive: algorithm's advantage from randomization is limited\n\n";

    // Part 5: Multiple trials showing randomness helps less
    std::cout << "Random vs LRU over 50 trials on adaptive sequence:\n\n";

    auto adseq2 = AdaptiveOnlineAdversary::adaptive_paging_adversary(
        lru_cache_state, k, alphabet, 100);

    double lru_r = static_cast<double>(deterministic_lru(k, adseq2, false))
                 / optimal_offline(k, adseq2, false);

    double rand_r_sum = 0;
    for (int t = 0; t < 50; t++)
        rand_r_sum += static_cast<double>(random_paging(k, adseq2, false))
                    / optimal_offline(k, adseq2, false);

    std::cout << "  LRU ratio:     " << std::setprecision(2) << lru_r << "\n";
    std::cout << "  Random ratio:  " << (rand_r_sum / 50.0) << " (avg over 50)\n";
    std::cout << "  Randomization provides smaller advantage against adaptive adversary\n";

    return 0;
}
