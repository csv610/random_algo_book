#include <iostream>
#include <string>
#include "markov_chain.h"
#include "random_walk.h"
#include "cover_time.h"

using namespace chapter6;

void print_header() {
    std::cout << "\n"
              << std::string(60, '=')
              << "\n"
              << "  CHAPTER 6: MARKOV CHAINS AND RANDOM WALKS\n"
              << std::string(60, '=')
              << "\n";
}

void print_section(int num, const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Section 6." << num << ": " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

int main() {
    print_header();

    print_section(2, "Markov Chains");
    demonstrate_markov_chain();

    print_section(3, "Random Walks on Graphs");
    demonstrate_random_walk();

    print_section(5, "Cover Times");
    demonstrate_cover_time();

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "SUMMARY OF CHAPTER 6\n";
    std::cout << std::string(60, '=') << "\n\n";

    std::cout << "Key Concepts Demonstrated:\n\n";

    std::cout << "1. MARKOV CHAINS\n";
    std::cout << "   - Transition matrix P, stationary distribution pi\n";
    std::cout << "   - Ergodic chains converge to pi regardless of start\n";
    std::cout << "   - Detailed balance implies reversibility\n\n";

    std::cout << "2. RANDOM WALKS ON GRAPHS\n";
    std::cout << "   - Stationary distribution pi_v = d_v / 2|E|\n";
    std::cout << "   - Mixing time bounded by 1/Phi^2 * log(n)\n";
    std::cout << "   - Conductance controls bottleneck severity\n\n";

    std::cout << "3. ELECTRICAL NETWORKS\n";
    std::cout << "   - Effective resistance R_eff(u,v)\n";
    std::cout << "   - Commute time: kappa(u,v) = 2|E| * R_eff(u,v)\n\n";

    std::cout << "4. COVER TIMES\n";
    std::cout << "   - T_cov <= 2|E| * (n-1) for any connected graph\n";
    std::cout << "   - Matthews: T_cov <= H_max * H_{n-1}\n";
    std::cout << "   - Star: Theta(n log n), Cycle: Theta(n^2)\n\n";

    std::cout << "5. EXPANDER GRAPHS\n";
    std::cout << "   - Phi(G) = Omega(1) => O(log n) mixing time\n";
    std::cout << "   - Expander mixing lemma bounds edge discrepancies\n";
    std::cout << "   - Probability amplification via expander walks\n\n";

    std::cout << "Compile and run:\n";
    std::cout << "  g++ -std=c++17 -O2 -o chapter6 main.cpp\n";
    std::cout << "  ./chapter6\n\n";

    return 0;
}
