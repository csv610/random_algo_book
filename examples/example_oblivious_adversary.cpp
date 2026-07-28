// example_oblivious_adversary.cpp
// Demonstrates the oblivious adversary model for online paging algorithms.
//
// An oblivious adversary fixes the entire request sequence before seeing
// the algorithm's responses. This models the worst-case input scenario.
//
// Key results:
//   - Any deterministic paging algorithm is at most k-competitive (k = cache size)
//   - Randomized algorithms achieve H_k-competitive ratio against oblivious adversaries
//   - The adversary cannot adapt to random choices, so randomization helps

#include "ral/paging.h"
#include "ral/adversary.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <numbers>

using namespace ral;

int main() {
    std::cout << "=== Oblivious Adversary for Online Paging ===\n\n";

    int k = 3;  // cache size

    // Part 1: Classic worst-case sequence for LRU/FIFO
    std::cout << "Classic adversarial sequence: cyclic permutation of k+1 pages\n";
    std::cout << "Cache size k = " << k << "\n\n";

    auto worst_seq = ObliviousAdversary::worst_case_lru_fifo(k, 12);
    std::cout << "  Sequence: ";
    for (int r : worst_seq) std::cout << r << " ";
    std::cout << "\n\n";

    int lru_misses = deterministic_lru(k, worst_seq, false);
    int fifo_misses = deterministic_fifo(k, worst_seq, false);
    int opt_misses = optimal_offline(k, worst_seq, false);
    int rand_misses = random_paging(k, worst_seq, false);

    std::cout << "  LRU:    " << lru_misses << " misses  ratio=" << std::setprecision(2)
              << static_cast<double>(lru_misses) / opt_misses << "\n";
    std::cout << "  FIFO:   " << fifo_misses << " misses  ratio="
              << static_cast<double>(fifo_misses) / opt_misses << "\n";
    std::cout << "  Random: " << rand_misses << " misses  ratio="
              << static_cast<double>(rand_misses) / opt_misses << " (one trial)\n";
    std::cout << "  OPT:    " << opt_misses << " misses\n\n";

    // Part 2: Why deterministic algorithms are k-competitive
    std::cout << "Why any deterministic algorithm is k-competitive:\n";
    std::cout << "  - Adversary knows algorithm's deterministic strategy\n";
    std::cout << "  - Adversary can always request a page not in cache\n";
    std::cout << "  - OPT keeps k pages cached; algorithm must miss at least 1 in k requests\n\n";

    // Part 3: Randomization helps against oblivious adversary
    std::cout << "Randomized algorithm vs oblivious adversary:\n";
    std::cout << "  - Average ratio over " << 50 << " trials\n\n";

    int seq_len = 200;
    auto oblivious_seq = ObliviousAdversary::worst_case_lru_fifo(k, seq_len);

    double det_ratio = static_cast<double>(deterministic_lru(k, oblivious_seq, false))
                     / optimal_offline(k, oblivious_seq, false);

    double rand_ratio_sum = 0;
    for (int t = 0; t < 50; t++)
        rand_ratio_sum += static_cast<double>(random_paging(k, oblivious_seq, false))
                        / optimal_offline(k, oblivious_seq, false);

    double hk = 0.0;
    for (int i = 1; i <= k; i++) hk += 1.0 / i;

    std::cout << "  Deterministic LRU ratio:  " << std::setprecision(2) << det_ratio
              << " (theory: k=" << k << ")\n";
    std::cout << "  Randomized avg ratio:     " << (rand_ratio_sum / 50.0)
              << " (theory: H_k=" << std::setprecision(4) << hk << ")\n";
    std::cout << "  Speedup: " << std::setprecision(2) << det_ratio / (rand_ratio_sum / 50.0)
              << "x\n\n";

    // Part 4: Different adversarial strategies
    std::cout << "Different oblivious adversarial strategies (k=" << k << "):\n\n";

    struct Strategy { std::string name; std::vector<int> seq; };
    std::vector<Strategy> strategies = {
        {"Cyclic", ObliviousAdversary::worst_case_lru_fifo(k, 100)},
        {"Interleaved", ObliviousAdversary::interleaved_adversary(k, 100)},
    };

    std::vector<int> uniform_seq(100);
    for (auto& s : uniform_seq) s = rng().rand_int(1, k + 1);
    strategies.push_back({"Uniform", uniform_seq});

    std::cout << std::setw(14) << "Strategy"
              << std::setw(12) << "LRU ratio"
              << std::setw(12) << "Random ratio"
              << "\n";
    std::cout << std::string(38, '-') << "\n";

    for (auto& [name, seq] : strategies) {
        auto opt = optimal_offline(k, seq, false);
        double lru_r = static_cast<double>(deterministic_lru(k, seq, false)) / opt;
        double rr = 0;
        for (int t = 0; t < 20; t++)
            rr += static_cast<double>(random_paging(k, seq, false)) / opt;
        rr /= 20;
        std::cout << std::setw(14) << name
                  << std::setw(12) << std::setprecision(2) << lru_r
                  << std::setw(12) << rr
                  << "\n";
    }

    std::cout << "\nSummary:\n";
    std::cout << "  - Oblivious adversary fixes sequence before algorithm runs\n";
    std::cout << "  - Deterministic algorithms are k-competitive (tight)\n";
    std::cout << "  - Randomization reduces ratio to H_k ~ ln(k)\n";
    std::cout << "  - Adversary cannot exploit random choices\n";

    return 0;
}
