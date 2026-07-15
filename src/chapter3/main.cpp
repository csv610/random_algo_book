#include <iostream>
#include "occupancy.h"
#include "inequalities.h"
#include "lazy_select.h"
#include "two_point.h"
#include "stable_marriage.h"
#include "coupon_collector.h"

using namespace chapter3;

void print_header() {
    std::cout << "+==============================================================+\n";
    std::cout << "|        RANDOMIZED ALGORITHMS - CHAPTER 3 IMPLEMENTATION    |\n";
    std::cout << "|        Based on Motwani & Raghavan's Textbook              |\n";
    std::cout << "+==============================================================+\n\n";
}

void print_section(int num, const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Section 3." << num << ": " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

int main() {
    print_header();

    // Section 3.1: Occupancy Problems
    print_section(1, "Occupancy Problems");
    demonstrate_occupancy();

    // Section 3.2: Markov and Chebyshev Inequalities
    print_section(2, "Markov and Chebyshev Inequalities");
    demonstrate_inequalities();

    // Section 3.3: Randomized Selection
    print_section(3, "Randomized Selection (LazySelect)");
    demonstrate_lazy_select();

    // Section 3.4: Two-Point Sampling
    print_section(4, "Two-Point Sampling");
    demonstrate_two_point();

    // Section 3.5: Stable Marriage Problem
    print_section(5, "Stable Marriage Problem");
    demonstrate_stable_marriage();

    // Section 3.6: Coupon Collector's Problem
    print_section(6, "Coupon Collector's Problem");
    demonstrate_coupon_collector();

    // Summary
    std::cout << std::string(60, '=') << "\n";
    std::cout << "SUMMARY OF CHAPTER 3\n";
    std::cout << std::string(60, '=') << "\n\n";

    std::cout << "Key Concepts Demonstrated:\n\n";

    std::cout << "1. OCCUPANCY PROBLEMS\n";
    std::cout << "   - Balls-in-bins: max load = O((en ln n)/(ln ln n))\n";
    std::cout << "   - Birthday problem: collision at m ~= sqrt(2n)\n";
    std::cout << "   - Union bound: Pr[A | B] <= Pr[A] + Pr[B]\n\n";

    std::cout << "2. TAIL INEQUALITIES\n";
    std::cout << "   - Markov: Pr[Y >= t] <= E[Y]/t\n";
    std::cout << "   - Chebyshev: Pr[|X-u| >= tsigma] <= 1/t^2\n";
    std::cout << "   - Second moment method: Pr[X>0] >= E[X]^2/(E[X]^2+Var(X))\n\n";

    std::cout << "3. LAZY SELECT\n";
    std::cout << "   - Randomized selection: expected 2n + o(n) comparisons\n";
    std::cout << "   - Beats deterministic 3n worst case\n";
    std::cout << "   - Succeeds w.h.p. on first pass\n\n";

    std::cout << "4. TWO-POINT SAMPLING\n";
    std::cout << "   - Probability amplification via pairwise independence\n";
    std::cout << "   - Error: 1/4 -> 1/t using O(log n) random bits\n";
    std::cout << "   - r_i = a*i + b mod n construction\n\n";

    std::cout << "5. STABLE MARRIAGE\n";
    std::cout << "   - Gale-Shapley: always terminates with stable matching\n";
    std::cout << "   - O(n^2) proposals worst case\n";
    std::cout << "   - Average case: O(n log n) via coupon collector\n\n";

    std::cout << "6. COUPON COLLECTOR\n";
    std::cout << "   - Expected trials: n*H_n ~= n ln n\n";
    std::cout << "   - Sharp threshold: Pr[X > n ln n + cn] -> 1 - e^{-e^{-c}}\n";
    std::cout << "   - Variance: O(n)\n\n";

    std::cout << "Compile and run:\n";
    std::cout << "  g++ -std=c++17 -O2 -o chapter3 main.cpp\n";
    std::cout << "  ./chapter3\n\n";

    return 0;
}
