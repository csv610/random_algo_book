#pragma once
// Two-Point Sampling: Probability amplification using pairwise independence
// Reduces error from 1/4 to 1/t using only O(log n) random bits

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>

namespace chapter4 {

// Simulated RP algorithm: for x in L, A(x,r)=1 for half the r's
// For x not in L, A(x,r)=0 for all r
bool RP_algorithm([[maybe_unused]] int x, int r, bool x_in_L, int n_half = 500) {
    if (!x_in_L) return false;
    return r < n_half;
}

// Naive: use a and b directly as witnesses. Error <= 1/4.
bool naive_two_point(int x, int n, bool x_in_L, std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, n - 1);
    int a = dist(rng), b = dist(rng);
    if (RP_algorithm(x, a, x_in_L)) return true;
    if (RP_algorithm(x, b, x_in_L)) return true;
    return false;
}

// Amplified: use r_i = a*i + b mod n for t samples
// Error <= 1/t by pairwise independence
bool amplified_two_point(int x, int n, int t, bool x_in_L,
                         std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, n - 1);
    int a = dist(rng), b = dist(rng);
    for (int i = 0; i < t; ++i) {
        int ri = (static_cast<long long>(a) * i + b) % n;
        if (RP_algorithm(x, ri, x_in_L)) return true;
    }
    return false;
}

// Demonstrate pairwise independence construction
void demonstrate_pairwise_independence(int n = 7, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, n - 1);

    std::cout << "Pairwise Independence Construction (Exercise 3.7)\n";
    std::cout << "  n = " << n << " (prime)\n";
    std::cout << "  Y_i = a*i + b mod n, for random a, b in Z_n\n\n";

    // Check pairwise independence
    int trials = 100000;
    std::vector<std::vector<int>> joint_count(n, std::vector<int>(n, 0));
    std::vector<int> marginal_count(n, 0);

    for (int t = 0; t < trials; ++t) {
        int a = dist(rng), b = dist(rng);
        int y0 = (a * 0 + b) % n;
        int y1 = (a * 1 + b) % n;
        ++joint_count[y0][y1];
        ++marginal_count[y0];
    }

    std::cout << "  Marginal distribution of Y_0 (should be uniform ~" << trials / n << "):\n  ";
    for (int i = 0; i < n; ++i)
        std::cout << std::setw(6) << marginal_count[i];
    std::cout << "\n\n";

    std::cout << "  Joint distribution P(Y_0=i, Y_1=j) (should be uniform ~" << trials / (n * n) << "):\n";
    for (int i = 0; i < n; ++i) {
        std::cout << "  ";
        for (int j = 0; j < n; ++j)
            std::cout << std::setw(6) << joint_count[i][j];
        std::cout << "\n";
    }
    std::cout << "\n";
}

void demonstrate_two_point() {
    std::cout << "Two-Point Sampling (Section 3.4)\n";
    std::cout << "Probability amplification using pairwise independence\n\n";

    int n = 1000;
    int trials = 100000;

    // Test with x in L (should almost always return true)
    std::mt19937 rng(42);

    std::cout << "Experiment: RP algorithm for x in L (correct answer = true)\n";
    std::cout << "  Naive (2 random bits): error = Pr[result = false]\n";
    std::cout << "  Amplified (O(log n) bits): error <= 1/t\n\n";

    std::cout << "  t     naive_error   amplified_error   1/t\n";

    for (int t : {2, 4, 8, 16, 32}) {
        int naive_false = 0, amp_false = 0;
        for (int i = 0; i < trials; ++i) {
            if (!naive_two_point(1, n, true, rng)) ++naive_false;
            if (!amplified_two_point(1, n, t, true, rng)) ++amp_false;
        }
        double naive_err = static_cast<double>(naive_false) / trials;
        double amp_err = static_cast<double>(amp_false) / trials;
        std::cout << "  " << std::setw(2) << t << "     "
                  << std::setw(8) << std::fixed << std::setprecision(4) << naive_err
                  << "        " << std::setw(8) << amp_err
                  << "       " << std::setw(6) << (1.0 / t) << "\n";
    }
    std::cout << std::defaultfloat << "\n";

    demonstrate_pairwise_independence();
}

}  // namespace chapter4
