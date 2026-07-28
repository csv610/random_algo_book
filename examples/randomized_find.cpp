#include "ral.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Example: Randomized Find (QuickSelect) - Chapter 1\n";
    std::cout << "========================================================\n\n";

    // Basic QuickSelect - using ral namespace
    std::cout << "--- QuickSelect (Las Vegas, O(n) expected) ---\n";
    std::vector<int> arr = {9, 7, 5, 11, 12, 2, 14, 3, 10, 6};
    std::cout << "Array: ";
    for (int x : arr) std::cout << x << " ";
    std::cout << "\n";

    for (int k = 1; k <= 10; k++) {
        std::vector<int> copy = arr;
        int result = ral::randomized_find(copy, k);
        std::cout << "  " << k << "-th smallest: " << result << "\n";
    }
    std::cout << "\n";

    // Performance analysis using chapter1 namespace
    std::cout << "--- Performance Analysis (n=1000, 100 trials) ---\n";
    int n = 1000;
    int trials = 100;
    long long total_calls = 0, total_depth = 0;

    for (int t = 0; t < trials; t++) {
        std::vector<int> data(n);
        for (int i = 0; i < n; i++) data[i] = ral::rng().rand_int(1, n * 10);
        int k = ral::rng().rand_int(1, n);

        ral::FindAnalysis analysis;
        ral::randomized_find_analyzed(data, k, 0, analysis);
        total_calls += analysis.total_calls;
        total_depth += analysis.max_depth;
    }

    std::cout << "  Avg recursive calls: " << (double)total_calls / trials << "\n";
    std::cout << "  Avg max depth: " << (double)total_depth / trials << "\n";
    std::cout << "  Theoretical bound: O(log n) = " << log2(n) << "\n\n";

    // Geometric distribution
    std::cout << "--- Geometric Distribution (Theorem 1.3) ---\n";
    double p = 0.3;
    int geo_trials = 100000;
    long long total_flips = 0;
    for (int t = 0; t < geo_trials; t++) {
        int flips = 0;
        while (true) {
            flips++;
            if (ral::rng().coin_flip(p)) break;
        }
        total_flips += flips;
    }
    std::cout << "  P(success) = " << p << "\n";
    std::cout << "  Expected flips: 1/p = " << 1.0/p << "\n";
    std::cout << "  Empirical avg: " << (double)total_flips / geo_trials << "\n\n";

    // Recursion depth bound
    std::cout << "--- Expected Recursion Depth Bound ---\n";
    for (int n : {100, 1000, 10000, 100000}) {
        double expected = log(n) / log(4.0/3.0);
        std::cout << "  n=" << n << ": expected depth <= " << expected << "\n";
    }

    return 0;
}