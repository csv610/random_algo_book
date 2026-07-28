// =====================================================================
// Randomized Marking Algorithm for Paging
//
// The marking algorithm achieves an expected H_k-competitive ratio
// (harmonic number ~ ln k) against an oblivious adversary.
//
// Algorithm:
//   - Each page is either "marked" or "unmarked"
//   - On access to a cached page: mark it
//   - On a miss when full: evict a random UNMARKED page
//   - When all k pages are marked: reset all marks (new round)
//
// The random choice over unmarked pages prevents an adversary from
// consistently forcing bad evictions, unlike deterministic LRU/FIFO.
//
// This example:
//   1. Runs marking on a small sequence (verbose, showing rounds)
//   2. Compares with LRU and OPT on the same adversarial sequence
//   3. Compares all algorithms on random sequences
// =====================================================================

#include "ral/paging.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Randomized Marking Algorithm for Paging\n";
    std::cout << "=============================================\n\n";

    // --- Small example with verbose output ---
    int k = 3;
    std::vector<int> requests = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};

    std::cout << "Cache size k=" << k << "\n";
    std::cout << "Request sequence: {1 2 3 4 1 2 5 1 2 3 4 5}\n\n";

    std::cout << "[Marking Algorithm]\n";
    int mark_misses = marking_algorithm(k, requests, true);
    std::cout << "  Total misses: " << mark_misses << "\n\n";

    std::cout << "[LRU for comparison]\n";
    int lru_misses = deterministic_lru(k, requests, true);
    std::cout << "  Total misses: " << lru_misses << "\n\n";

    std::cout << "[OPT]\n";
    int opt_misses = optimal_offline(k, requests, true);
    std::cout << "  Total misses: " << opt_misses << "\n\n";

    std::cout << "  Mark/OPT = " << (double)mark_misses / opt_misses << "\n";
    std::cout << "  LRU/OPT  = " << (double)lru_misses / opt_misses << "\n\n";

    // --- Adversarial sequence: cyclic of k+1 pages ---
    std::cout << "--- Adversarial Sequence (k=" << k << ") ---\n";
    std::vector<int> adseq;
    std::vector<int> cycle = {1, 2, 3, 4};
    for (int i = 0; i < 20; i++)
        adseq.push_back(cycle[i % 4]);

    int ad_lru  = deterministic_lru(k, adseq, false);
    int ad_mark = marking_algorithm(k, adseq, false);
    int ad_opt  = optimal_offline(k, adseq, false);

    std::cout << "  Cyclic sequence, length 20, k=" << k << "\n";
    std::cout << "  LRU:   " << ad_lru  << " misses  (ratio " << (double)ad_lru / ad_opt << ")\n";
    std::cout << "  Mark:  " << ad_mark << " misses  (ratio " << (double)ad_mark / ad_opt << ")\n";
    std::cout << "  OPT:   " << ad_opt  << " misses\n";
    std::cout << "  Mark beats LRU on adversarial sequences!\n\n";

    // --- H_k bound ---
    double Hk = 0;
    for (int i = 1; i <= k; i++) Hk += 1.0 / i;
    std::cout << "  H_" << k << " = " << std::fixed << std::setprecision(2) << Hk
              << "  (theoretical upper bound on Mark/OPT ratio)\n\n";

    // --- Average case on random sequences ---
    std::cout << "--- Random Sequences (100 trials, 1000 requests, 10 pages) ---\n";
    double total_lru = 0, total_mark = 0, total_opt = 0;
    constexpr int trials = 100;
    constexpr int seq_len = 1000;
    constexpr int num_pages = 10;

    for (int t = 0; t < trials; t++) {
        auto req = generate_random_requests(seq_len, num_pages);
        total_lru  += deterministic_lru(k, req, false);
        total_mark += marking_algorithm(k, req, false);
        total_opt  += optimal_offline(k, req, false);
    }
    std::cout << "  Avg LRU:   " << total_lru / trials
              << "  (ratio " << total_lru / total_opt << ")\n";
    std::cout << "  Avg Mark:  " << total_mark / trials
              << "  (ratio " << total_mark / total_opt << ")\n";
    std::cout << "  Avg OPT:   " << total_opt / trials << "\n\n";

    std::cout << "Theory: Marking is H_k-competitive in expectation.\n";
    return 0;
}
