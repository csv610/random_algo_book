#include <iostream>
#include <string>
#include "chernoff.h"
#include "routing.h"
#include "martingales.h"

using namespace chapter5;

void print_header() {
    std::cout << "+==============================================================+\n";
    std::cout << "|        RANDOMIZED ALGORITHMS - CHAPTER 4 IMPLEMENTATION    |\n";
    std::cout << "|        Tail Inequalities                                    |\n";
    std::cout << "+==============================================================+\n\n";
}

void print_section(int num, const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Section 4." << num << ": " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

int main() {
    print_header();

    // Section 4.1: The Chernoff Bound
    print_section(1, "The Chernoff Bound");
    demonstrate_chernoff();

    // Section 4.2: Routing in a Parallel Computer
    print_section(2, "Routing in a Parallel Computer");
    demonstrate_routing();

    // Section 4.4: Martingales
    print_section(4, "Martingales");
    demonstrate_martingales();

    // Summary
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "SUMMARY OF CHAPTER 4\n";
    std::cout << std::string(60, '=') << "\n\n";

    std::cout << "Key Concepts Demonstrated:\n\n";

    std::cout << "1. CHERNOFF BOUNDS\n";
    std::cout << "   - MGF-based technique: Markov's inequality on e^{lambda*X}\n";
    std::cout << "   - Upper tail: Pr[X >= (1+d)*mu] <= exp(-mu*d^2/3) for d <= 1\n";
    std::cout << "   - Lower tail: Pr[X <= (1-d)*mu] <= exp(-mu*d^2/2) for d <= 1\n";
    std::cout << "   - Both bounds are exponentially small in mu\n\n";

    std::cout << "2. ROUTING IN PARALLEL COMPUTERS\n";
    std::cout << "   - Random routing achieves O(ln n / ln ln n) max congestion\n";
    std::cout << "   - Chernoff bound + union bound gives high-probability guarantee\n";
    std::cout << "   - Experimentally verified for various n\n\n";

    std::cout << "3. MARTINGALES AND AZUMA-HOEFFDING\n";
    std::cout << "   - Martingales model fair games: E[X_i | past] = X_{i-1}\n";
    std::cout << "   - Azuma-Hoeffding: Pr[|X_n - X_0| >= t] <= 2*exp(-t^2/(2*sum(c_i^2)))\n";
    std::cout << "   - Bounded differences: changing one input changes output by <= c_i\n";
    std::cout << "   - Doob martingale connects function concentration to martingale theory\n\n";

    std::cout << "Compile and run:\n";
    std::cout << "  g++ -std=c++17 -O2 -o chapter4 main.cpp\n";
    std::cout << "  ./chapter4\n\n";

    return 0;
}
