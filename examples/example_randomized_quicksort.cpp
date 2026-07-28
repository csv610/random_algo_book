// example_randomized_quicksort.cpp
// Demonstrates Las Vegas randomized QuickSort: the output is always correct,
// but the running time is a random variable with E[T(n)] = O(n log n).
//
// A Las Vegas algorithm always produces the correct result. Its running time
// may vary across runs, but has good expected (and high-probability) bounds.
// This is contrasted with Monte Carlo algorithms which have fixed running
// time but may produce incorrect results with small probability.

#include "ral/las_vegas_monte_carlo.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <algorithm>

using namespace ral;

int main() {
    std::cout << "=== Las Vegas Algorithm: Randomized QuickSort ===\n\n";

    // Sort arrays of various sizes, measure time, verify correctness
    std::cout << std::setw(10) << "n"
              << std::setw(14) << "time (us)"
              << std::setw(10) << "sorted?"
              << std::setw(14) << "n*lg(n)/2"
              << "\n";
    std::cout << std::string(48, '-') << "\n";

    for (int n : {10, 100, 1000, 5000, 10000}) {
        std::vector<int> arr(n);
        for (int i = 0; i < n; i++)
            arr[i] = rng().rand_int(1, n * 10);

        auto start = std::chrono::high_resolution_clock::now();
        randomized_quicksort(arr, 0, n - 1);
        auto end = std::chrono::high_resolution_clock::now();

        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        bool sorted = std::is_sorted(arr.begin(), arr.end());

        std::cout << std::setw(10) << n
                  << std::setw(14) << us.count()
                  << std::setw(10) << (sorted ? "YES" : "NO")
                  << std::setw(14) << static_cast<int>(0.5 * n * std::log2(n))
                  << "\n";
    }

    // Run 10 trials on the same input to show variance
    std::cout << "\nVariability across runs (n=5000, same input):\n";
    {
        std::vector<int> original(5000);
        for (int i = 0; i < 5000; i++)
            original[i] = rng().rand_int(1, 50000);

        long long total_us = 0;
        for (int trial = 0; trial < 10; trial++) {
            std::vector<int> arr = original;
            auto start = std::chrono::high_resolution_clock::now();
            randomized_quicksort(arr, 0, arr.size() - 1);
            auto end = std::chrono::high_resolution_clock::now();
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            total_us += us.count();
            std::cout << "  Trial " << std::setw(2) << trial + 1
                      << ": " << us.count() << " us"
                      << "  sorted: " << (std::is_sorted(arr.begin(), arr.end()) ? "YES" : "NO")
                      << "\n";
        }
        std::cout << "  Average: " << (total_us / 10) << " us\n";
    }

    std::cout << "\nLas Vegas properties:\n";
    std::cout << "  - Output is always correct\n";
    std::cout << "  - Running time is random but E[T(n)] = O(n log n)\n";
    std::cout << "  - Worst case is O(n^2) but with exponentially small probability\n";
    std::cout << "  - High-probability bound: T(n) = O(n log n) with probability >= 1 - 1/n^c\n";

    return 0;
}
