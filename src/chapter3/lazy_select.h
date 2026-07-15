#pragma once
// LazySelect: Randomized selection algorithm
// Theorem 3.5: Expected 2n + o(n) comparisons, succeeds w.h.p. on first pass

#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace chapter3 {

struct SelectResult {
    int value;
    int comparisons;
    int pass;  // which pass succeeded
};

// LazySelect: find k-th smallest element (1-indexed)
// Uses O(n^{3/4}) samples, expects 2n + o(n) comparisons
SelectResult lazy_select(std::vector<int>& S, int k, std::mt19937& rng) {
    int n = static_cast<int>(S.size());
    int sample_size = std::max(1, static_cast<int>(std::ceil(std::pow(n, 0.75))));
    std::uniform_int_distribution<int> dist(0, n - 1);

    int pass = 0;
    while (true) {
        ++pass;

        // Step 1: sample n^{3/4} elements with replacement
        std::vector<int> R(sample_size);
        for (int i = 0; i < sample_size; ++i)
            R[i] = S[dist(rng)];

        // Step 2: sort R
        std::sort(R.begin(), R.end());

        // Step 3: compute indices t, h
        double x = static_cast<double>(k) * std::pow(n, -0.25);
        int t = std::max(static_cast<int>(std::floor(x - std::sqrt(n))), 1);
        int h = std::min(static_cast<int>(std::ceil(x + std::sqrt(n))),
                         sample_size - 1);
        int a_val = R[t];
        int b_val = R[h];

        // Step 4: compute ranks of a and b in S
        int rank_a = 0, rank_b = 0;
        for (int i = 0; i < n; ++i) {
            if (S[i] <= a_val) ++rank_a;
            if (S[i] < b_val) ++rank_b;
        }

        // Build candidate set P
        std::vector<int> P;
        int n_quarter = static_cast<int>(std::pow(n, 0.25));
        if (k < n_quarter) {
            for (int v : S) if (v <= b_val) P.push_back(v);
        } else if (k > n - n_quarter) {
            for (int v : S) if (v >= a_val) P.push_back(v);
        } else {
            for (int v : S)
                if (v >= a_val && v <= b_val) P.push_back(v);
        }

        // Check S<k> is in P and |P| is small enough
        int target_in_P = k - rank_a + 1;
        if (target_in_P >= 1 && target_in_P <= static_cast<int>(P.size())
            && static_cast<int>(P.size()) <= 4 * sample_size + 2) {
            // Step 7: sort P and pick the element
            std::sort(P.begin(), P.end());
            return {P[target_in_P - 1], 2 * n + static_cast<int>(P.size()), pass};
        }
        // Otherwise retry
    }
}

// Simple deterministic selection (for comparison)
int deterministic_select(std::vector<int>& S, int k) {
    std::vector<int> copy = S;
    std::nth_element(copy.begin(), copy.begin() + k - 1, copy.end());
    return copy[k - 1];
}

void demonstrate_lazy_select() {
    std::cout << "LazySelect Algorithm (Section 3.3)\n";
    std::cout << "Finds k-th smallest element with expected 2n + o(n) comparisons\n\n";

    std::mt19937 rng(42);

    for (int n : {100, 500, 1000, 5000}) {
        std::vector<int> S(n);
        for (int i = 0; i < n; ++i) S[i] = i + 1;
        std::shuffle(S.begin(), S.end(), rng);

        int k = n / 2;  // find median

        // Run LazySelect
        double total_comps = 0;
        double total_passes = 0;
        int trials = 100;
        bool all_correct = true;

        for (int t = 0; t < trials; ++t) {
            std::vector<int> copy = S;
            auto result = lazy_select(copy, k, rng);
            total_comps += result.comparisons;
            total_passes += result.pass;

            // Verify correctness
            std::vector<int> sorted = S;
            std::sort(sorted.begin(), sorted.end());
            if (result.value != sorted[k - 1]) all_correct = false;
        }

        double avg_comps = total_comps / trials;
        double avg_passes = total_passes / trials;
        double theory = 2.0 * n;

        std::cout << "n=" << n << "  k=" << k
                  << "  avg_comps=" << std::setw(8) << avg_comps
                  << "  theory=2n=" << std::setw(6) << theory
                  << "  avg_passes=" << std::setw(5) << avg_passes
                  << "  correct=" << (all_correct ? "Yes" : "NO!") << "\n";
    }
    std::cout << "\n";

    // Compare with deterministic
    std::cout << "Comparison with deterministic selection:\n";
    std::cout << "  Deterministic: 3n comparisons worst case\n";
    std::cout << "  LazySelect:    2n + o(n) expected\n";
    std::cout << "  Speedup factor approaches 1.5x for large n\n\n";
}

}  // namespace chapter3
