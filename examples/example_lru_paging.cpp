// =====================================================================
// LRU (Least Recently Used) Deterministic Paging
//
// Paging problem: maintain k pages in cache, serve a sequence of
// page requests. On a miss, evict one page. Goal: minimize misses.
//
// LRU evicts the page that was accessed least recently.
//   - k-competitive against an oblivious adversary (tight bound)
//   - On the adversarial cyclic sequence of (k+1) pages,
//     LRU misses every other request (ratio = k).
//
// This example:
//   1. Runs LRU on a small hand-crafted sequence (verbose)
//   2. Builds an adversarial sequence to show the k-competitive bound
//   3. Compares LRU vs OPT on random sequences
// =====================================================================

#include "ral/paging.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  LRU Deterministic Paging\n";
    std::cout << "=============================================\n\n";

    // --- Small example with verbose output ---
    int k = 3;
    std::vector<int> requests = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};

    std::cout << "Cache size k=" << k << "\n";
    std::cout << "Request sequence: {1 2 3 4 1 2 5 1 2 3 4 5}\n\n";

    std::cout << "[LRU]\n";
    int lru_misses = deterministic_lru(k, requests, true);
    std::cout << "  Total misses: " << lru_misses << "\n\n";

    std::cout << "[OPT (Belady's)]\n";
    int opt_misses = optimal_offline(k, requests, true);
    std::cout << "  Total misses: " << opt_misses << "\n\n";

    std::cout << "LRU/OPT ratio = " << (double)lru_misses / opt_misses << "\n\n";

    // --- Adversarial sequence: forces LRU to miss every other request ---
    std::cout << "--- Adversarial Sequence (k=" << k << ") ---\n";
    std::vector<int> adseq;
    std::vector<int> cycle = {1, 2, 3, 4};  // k+1 distinct pages
    for (int i = 0; i < 20; i++)
        adseq.push_back(cycle[i % 4]);

    int ad_lru = deterministic_lru(k, adseq, false);
    int ad_opt = optimal_offline(k, adseq, false);
    std::cout << "  Cyclic sequence of 4 pages, length 20\n";
    std::cout << "  LRU misses: " << ad_lru << "\n";
    std::cout << "  OPT misses: " << ad_opt << "\n";
    std::cout << "  Ratio:      " << (double)ad_lru / ad_opt
              << "  (theory: k=" << k << ")\n\n";

    // --- Compare on random sequences ---
    std::cout << "--- Random Sequences (100 trials, 1000 requests, 10 pages) ---\n";
    double total_lru = 0, total_opt = 0;
    constexpr int trials = 100;
    constexpr int seq_len = 1000;
    constexpr int num_pages = 10;

    for (int t = 0; t < trials; t++) {
        auto req = generate_random_requests(seq_len, num_pages);
        total_lru += deterministic_lru(k, req, false);
        total_opt += optimal_offline(k, req, false);
    }
    std::cout << "  Avg LRU misses: " << total_lru / trials << "\n";
    std::cout << "  Avg OPT misses: " << total_opt / trials << "\n";
    std::cout << "  Avg ratio:      " << total_lru / total_opt << "\n\n";

    std::cout << "Theory: LRU is k-competitive, OPT is optimal offline.\n";
    return 0;
}
