// =====================================================================
// Lazy Selection Algorithm (Randomized Order Statistics)
//
// LazySelect finds the k-th smallest element in an unsorted array
// using expected 2n + o(n) comparisons.
//
// Algorithm:
//   1. Sample n^{3/4} elements with replacement, sort them
//   2. Pick two sample elements a = R[t], b = R[h] as rank bounds
//   3. Scan S once to compute exact ranks of a and b  [2n work]
//   4. If the candidate set P is small enough (O(n^{3/4})), sort P
//   5. Otherwise retry (succeeds w.h.p. on first pass)
//
// Comparison:
//   - Deterministic median-of-medians: 3n comparisons worst case
//   - LazySelect: 2n + o(n) expected, succeeds w.h.p. first try
//   - Plain quickselect: O(n) expected but O(n^2) worst case
// =====================================================================

#include "ral/lazy_select.h"
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Lazy Selection Algorithm\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    // --- Single demo ---
    int n = 1000;
    std::vector<int> S(n);
    for (int i = 0; i < n; i++) S[i] = i + 1;
    std::shuffle(S.begin(), S.end(), rng);

    int k = n / 2;  // find median

    // Verify with deterministic
    std::vector<int> sorted = S;
    std::sort(sorted.begin(), sorted.end());
    int correct_answer = sorted[k - 1];

    std::cout << "Finding k=" << k << "-th smallest (median) in n=" << n << " elements\n\n";

    // Run LazySelect
    std::vector<int> copy = S;
    auto result = lazy_select(copy, k, rng);

    std::cout << "LazySelect result: " << result.value << "\n";
    std::cout << "Correct answer:    " << correct_answer << "\n";
    std::cout << "Match: " << (result.value == correct_answer ? "YES" : "NO!") << "\n";
    std::cout << "Pass:  " << result.pass << "\n";
    std::cout << "Comparisons: " << result.comparisons << "  (2n=" << 2 * n << ")\n\n";

    // --- Comparison across sizes ---
    std::cout << "--- LazySelect vs Deterministic (100 trials each) ---\n";
    std::cout << std::setw(8) << "n"
              << std::setw(12) << "lazy_comps"
              << std::setw(12) << "det_comps"
              << std::setw(10) << "ratio"
              << std::setw(10) << "avg_pass"
              << "\n";

    for (int sz : {100, 500, 1000, 5000}) {
        double total_lazy = 0, total_pass = 0;
        int total_det = 0;
        constexpr int trials = 100;

        std::vector<int> base(sz);
        for (int i = 0; i < sz; i++) base[i] = i + 1;

        for (int t = 0; t < trials; t++) {
            std::mt19937 trial_rng(t * 54321 + sz);
            std::vector<int> arr = base;
            std::shuffle(arr.begin(), arr.end(), trial_rng);

            int kk = sz / 2;
            std::vector<int> copy2 = arr;
            auto res = lazy_select(copy2, kk, trial_rng);
            total_lazy += res.comparisons;
            total_pass += res.pass;

            std::vector<int> arr2 = arr;
            deterministic_select(arr2, kk);
            total_det += 3 * sz;
        }

        double avg_lazy = total_lazy / trials;
        double avg_pass = total_pass / trials;
        double avg_det = (double)total_det / trials;

        std::cout << std::setw(8) << sz
                  << std::setw(12) << std::fixed << std::setprecision(0) << avg_lazy
                  << std::setw(12) << std::fixed << std::setprecision(0) << avg_det
                  << std::setw(10) << std::fixed << std::setprecision(2) << avg_lazy / avg_det
                  << std::setw(10) << std::fixed << std::setprecision(2) << avg_pass
                  << "\n";
    }

    std::cout << "\nTheory: LazySelect = 2n+o(n), Deterministic = 3n. Ratio -> 2/3.\n";
    return 0;
}
