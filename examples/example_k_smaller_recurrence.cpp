// example_k_smaller_recurrence.cpp
// Demonstrates the "k-adding" probabilistic recurrence for randomized selection.
//
// The randomized Find algorithm selects the k-th smallest element in expected
// O(n) time by recursively working on a fraction of the input. At each step,
// a random pivot splits the array, and the expected size of the subproblem
// reduces by at least 1/4 of the current size.
//
// By Theorem 1.3 (probabilistic recurrence): if E[work at step m] >= g(m),
// then expected total steps <= integral from 1 to n of dx/g(x).
// For Find: g(x) = x/4, so the bound is 4*ln(n).

#include "ral/probabilistic_recurrence.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>

using namespace ral;

int main() {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "=== Probabilistic Recurrence: K-Smaller / Randomized Find ===\n\n";

    // Part 1: Run randomized find and measure recursion depth
    std::cout << "Randomized Find: select k-th smallest in expected O(n)\n\n";
    std::cout << std::setw(8) << "n"
              << std::setw(14) << "avg_calls"
              << std::setw(14) << "avg_depth"
              << std::setw(14) << "4*ln(n)"
              << "\n";
    std::cout << std::string(50, '-') << "\n";

    for (int n : {100, 500, 1000, 5000}) {
        int num_trials = 200;
        long long total_calls = 0;
        long long total_depth = 0;

        for (int trial = 0; trial < num_trials; trial++) {
            std::vector<int> arr(n);
            for (int i = 0; i < n; i++)
                arr[i] = rng().rand_int(1, n * 10);

            int k = rng().rand_int(1, n);

            FindAnalysis analysis;
            randomized_find_analyzed(arr, k, 0, analysis);

            total_calls += analysis.total_calls;
            total_depth += analysis.max_depth;
        }

        double avg_calls = static_cast<double>(total_calls) / num_trials;
        double avg_depth = static_cast<double>(total_depth) / num_trials;

        std::cout << std::setw(8) << n
                  << std::setw(14) << avg_calls
                  << std::setw(14) << avg_depth
                  << std::setw(14) << 4.0 * std::log(static_cast<double>(n))
                  << "\n";
    }

    // Part 2: Geometric distribution connection
    std::cout << "\nGeometric Distribution (coin flips until first heads)\n";
    std::cout << "P(heads) = p, E[flips] = 1/p\n\n";

    std::cout << std::setw(10) << "p"
              << std::setw(14) << "1/p"
              << std::setw(14) << "simulated"
              << "\n";
    std::cout << std::string(38, '-') << "\n";

    for (double p : {0.1, 0.2, 0.25, 0.3, 0.5, 0.75}) {
        long long total_flips = 0;
        int num_trials = 100000;
        for (int t = 0; t < num_trials; t++) {
            int flips = 0;
            while (true) {
                flips++;
                if (rng().coin_flip(p)) break;

            }
            total_flips += flips;
        }
        double avg = static_cast<double>(total_flips) / num_trials;
        std::cout << std::setw(10) << p
                  << std::setw(14) << (1.0 / p)
                  << std::setw(14) << avg
                  << "\n";
    }

    // Part 3: Recursion depth analysis
    std::cout << "\nRecursion depth for Find algorithm:\n";
    std::cout << "  Problem reduces by E[X] >= n/4 each step\n";
    std::cout << "  Expected depth <= log(n)/log(4/3)\n\n";

    std::cout << std::setw(10) << "n"
              << std::setw(14) << "log(n)/log(4/3)"
              << "\n";
    std::cout << std::string(24, '-') << "\n";

    for (int n : {100, 1000, 10000, 100000}) {
        std::cout << std::setw(10) << n
                  << std::setw(14) << std::log(static_cast<double>(n)) / std::log(4.0 / 3.0)
                  << "\n";
    }

    std::cout << "\nKey result: Expected total work = O(n)\n";
    std::cout << "  Each level does O(n) work across all branches\n";
    std::cout << "  O(log n) levels of recursion -> O(n) total\n";

    return 0;
}
