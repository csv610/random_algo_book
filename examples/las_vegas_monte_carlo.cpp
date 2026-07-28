#include "ral.h"
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Example: Las Vegas vs Monte Carlo (Chapter 1)\n";
    std::cout << "========================================================\n\n";

    // Las Vegas: Randomized QuickSort - always correct, random runtime
    std::cout << "--- Las Vegas: Randomized QuickSort ---\n";
    std::vector<int> arr = {9, 7, 5, 11, 12, 2, 14, 3, 10, 6};
    std::cout << "Input: ";
    for (int x : arr) std::cout << x << " ";
    std::cout << "\n";

    ral::randomized_quicksort(arr, 0, arr.size() - 1);
    std::cout << "Sorted: ";
    for (int x : arr) std::cout << x << " ";
    std::cout << "\n\n";

    // Monte Carlo: Pi estimation - may be wrong, bounded error
    std::cout << "--- Monte Carlo: Pi Estimation ---\n";
    for (int samples : {100, 1000, 10000, 100000, 1000000}) {
        double pi = ral::estimate_pi_monte_carlo(samples);
        std::cout << "  Samples: " << samples << ", Pi ~ " << pi 
                  << " (error: " << std::abs(pi - 3.14159265359) << ")\n";
    }
    std::cout << "\n";

    // Monte Carlo: Min-Cut (one-sided error) using edge list format
    std::cout << "--- Monte Carlo: Min-Cut (one-sided error) ---\n";
    std::vector<std::pair<int, int>> edges = {
        {0, 1}, {0, 2}, {0, 3},
        {1, 2}, {1, 4},
        {2, 3}, {2, 4},
        {3, 5}, {4, 5}
    };

    for (int trials : {1, 5, 10, 50, 100}) {
        int cut = ral::randomized_min_cut_monte_carlo(edges, 6, trials);
        std::cout << "  Trials: " << trials << ", Min-cut found: " << cut << "\n";
    }
    std::cout << "\n";

    // Geometric distribution demo
    std::cout << "--- Geometric Distribution (Monte Carlo -> Las Vegas) ---\n";
    std::cout << "Monte Carlo with p=2/3 success prob.\n";
    std::cout << "Expected attempts to get correct answer: 1/(2/3) = 1.5\n";
    std::cout << "Las Vegas version: repeats until verified correct\n\n";

    return 0;
}