// example_linear_expectation.cpp
// Demonstrates linearity of expectation applied to recurrences.
//
// The key technique: express E[T(n)] in terms of E[T(smaller problems)]
// by conditioning on the random pivot. For randomized QuickSelect:
//   E[T(n)] = n + (1/n) * sum_{j=1}^{n} E[T(max(j-1, n-j))]
//
// Solving this recurrence gives E[T(n)] = O(n) for QuickSelect,
// and E[T(n)] = O(n log n) for QuickSort.
//
// We also demonstrate the indicator variable technique:
// E[total cost] = sum E[cost of element i's contribution]

#include "ral/probabilistic_recurrence.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

using namespace ral;

// Simulate QuickSort comparisons
int quicksort_comparisons(std::vector<int> arr) {
    if (arr.size() <= 1) return 0;
    int comparisons = 0;

    auto qs = [&](auto&& self, int lo, int hi) -> void {
        if (lo >= hi) return;
        int pivot_idx = rng().rand_int(lo, hi);
        int pivot = arr[pivot_idx];
        std::swap(arr[pivot_idx], arr[hi]);
        int i = lo;
        for (int j = lo; j < hi; j++) {
            comparisons++;
            if (arr[j] <= pivot) {
                std::swap(arr[i], arr[j]);
                i++;
            }
        }
        std::swap(arr[i], arr[hi]);
        self(self, lo, i - 1);
        self(self, i + 1, hi);
    };

    qs(qs, 0, static_cast<int>(arr.size()) - 1);
    return comparisons;
}

int main() {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "=== Linearity of Expectation in Recurrences ===\n\n";

    // Part 1: QuickSort comparisons
    // E[comparisons] = 2*n*H_n - 2*n (approximately)
    std::cout << "QuickSort: E[comparisons] ~ 2*n*ln(n)\n\n";
    std::cout << std::setw(8) << "n"
              << std::setw(14) << "sim_avg_comp"
              << std::setw(14) << "2*n*ln(n)"
              << std::setw(10) << "ratio"
              << "\n";
    std::cout << std::string(46, '-') << "\n";

    for (int n : {50, 100, 500, 1000, 5000}) {
        long long total_comp = 0;
        int num_trials = 500;
        for (int t = 0; t < num_trials; t++) {
            std::vector<int> arr(n);
            for (int i = 0; i < n; i++) arr[i] = rng().rand_int(1, n);
            total_comp += quicksort_comparisons(arr);
        }
        double avg = static_cast<double>(total_comp) / num_trials;
        double theory = 2.0 * n * std::log(static_cast<double>(n));
        std::cout << std::setw(8) << n
                  << std::setw(14) << avg
                  << std::setw(14) << theory
                  << std::setw(10) << std::setprecision(3) << (avg / theory)
                  << std::setprecision(2) << "\n";
    }

    // Part 2: QuickSelect - expected comparisons for finding median
    // E[T(n)] = 2*n + O(1) for selecting the median
    std::cout << "\nQuickSelect: E[comparisons to find median] ~ 2*n\n\n";
    std::cout << std::setw(8) << "n"
              << std::setw(14) << "sim_avg_comp"
              << std::setw(14) << "2*n"
              << std::setw(10) << "ratio"
              << "\n";
    std::cout << std::string(46, '-') << "\n";

    for (int n : {50, 100, 500, 1000, 5000}) {
        long long total_comp = 0;
        int num_trials = 500;
        for (int t = 0; t < num_trials; t++) {
            std::vector<int> arr(n);
            for (int i = 0; i < n; i++) arr[i] = rng().rand_int(1, n * 10);

            // Run randomized find and count comparisons
            std::vector<int> work = arr;
            int k = n / 2;  // median
            int comps = 0;
            while (work.size() > 1) {
                int pivot_idx = rng().rand_int(0, static_cast<int>(work.size()) - 1);
                int pivot = work[pivot_idx];
                std::vector<int> less, equal, greater;
                for (int x : work) {
                    comps++;
                    if (x < pivot) less.push_back(x);
                    else if (x == pivot) equal.push_back(x);
                    else greater.push_back(x);
                }
                if (k <= static_cast<int>(less.size())) {
                    work = std::move(less);
                } else if (k <= static_cast<int>(less.size() + equal.size())) {
                    break;
                } else {
                    k -= static_cast<int>(less.size() + equal.size());
                    work = std::move(greater);
                }
            }
            total_comp += comps;
        }
        double avg = static_cast<double>(total_comp) / num_trials;
        std::cout << std::setw(8) << n
                  << std::setw(14) << avg
                  << std::setw(14) << (2.0 * n)
                  << std::setw(10) << std::setprecision(3) << (avg / (2.0 * n))
                  << std::setprecision(2) << "\n";
    }

    // Part 3: Indicator variable technique
    // In a random permutation, E[adjacent inversions] = 1/2 per pair
    std::cout << "\nIndicator technique: Inversions in random permutation\n";
    std::cout << "E[inversions] = C(n,2)/2\n\n";

    std::cout << std::setw(8) << "n"
              << std::setw(14) << "sim_avg_inv"
              << std::setw(14) << "C(n,2)/2"
              << std::setw(10) << "ratio"
              << "\n";
    std::cout << std::string(46, '-') << "\n";

    for (int n : {10, 50, 100, 500, 1000}) {
        long long total_inv = 0;
        int num_trials = 5000;
        for (int t = 0; t < num_trials; t++) {
            std::vector<int> perm(n);
            std::iota(perm.begin(), perm.end(), 0);
            rng().shuffle(perm);
            int inv = 0;
            for (int i = 0; i < n; i++)
                for (int j = i + 1; j < n; j++)
                    if (perm[i] > perm[j]) inv++;
            total_inv += inv;
        }
        double avg = static_cast<double>(total_inv) / num_trials;
        double theory = 0.5 * n * (n - 1) / 2.0;
        std::cout << std::setw(8) << n
                  << std::setw(14) << avg
                  << std::setw(14) << theory
                  << std::setw(10) << std::setprecision(3) << (avg / theory)
                  << std::setprecision(2) << "\n";
    }

    std::cout << "\nKey insight: Linearity of expectation lets us analyze\n";
    std::cout << "complex recurrences by decomposing into simple sub-expectations.\n";

    return 0;
}
