#include <iostream>
#include <string>
#include "maxsat.h"
#include "expander.h"
#include "lovasz_ll.h"

using namespace chapter5;

void print_header() {
    std::cout << "\xc2\x9b\xc2\x90\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\xc2\x9b\xc2\x80\xc2\x94\n";
}

void print_section(int num, const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Section 5." << num << ": " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

int main() {
    print_header();

    // Section 5.2: Maximum Satisfiability
    print_section(2, "Maximum Satisfiability");
    demonstrate_maxsat();

    // Section 5.3: Expanding Graphs
    print_section(3, "Expanding Graphs");
    demonstrate_expander();

    // Section 5.5: The Lovasz Local Lemma
    print_section(5, "The Lovasz Local Lemma");
    demonstrate_lovasz();

    // Summary
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "SUMMARY OF CHAPTER 5\n";
    std::cout << std::string(60, '=') << "\n\n";

    std::cout << "Key Concepts Demonstrated:\n\n";

    std::cout << "1. PROBABILISTIC METHOD\n";
    std::cout << "   - Prove existence by showing positive probability\n";
    std::cout << "   - Linearity of expectation: E[sum X_i] = sum E[X_i]\n";
    std::cout << "   - Expected monochromatic subgraphs\n\n";

    std::cout << "2. MAXIMUM SATISFIABILITY\n";
    std::cout << "   - Random assignment satisfies >= m/2 clauses\n";
    std::cout << "   - 7/8-approximation for MAX-3SAT via random assignment\n";
    std::cout << "   - Deterministic algorithm via conditional expectations\n\n";

    std::cout << "3. EXPANDER GRAPHS\n";
    std::cout << "   - Random d-regular graphs are expanders w.h.p.\n";
    std::cout << "   - Spectral gap controls expansion (Cheeger inequality)\n";
    std::cout << "   - Ramanujan bound: |lambda_2| <= 2*sqrt(d-1)\n\n";

    std::cout << "4. OBLIVIOUS ROUTING\n";
    std::cout << "   - Random shortest-path routing on general graphs\n";
    std::cout << "   - Competitive ratio O(log n) via Chernoff + union bound\n\n";

    std::cout << "5. LOVASZ LOCAL LEMMA\n";
    std::cout << "   - If events have limited dependency, all can be avoided\n";
    std::cout << "   - Symmetric condition: ep(d+1) <= 1\n";
    std::cout << "   - Applications: 2-SAT, hypergraph coloring, k-SAT\n\n";

    std::cout << "6. METHOD OF CONDITIONAL PROBABILITIES\n";
    std::cout << "   - Derandomization by fixing variables greedily\n";
    std::cout << "   - Maintains E[X | choices so far] >= E[X]\n";
    std::cout << "   - Yields deterministic polynomial-time algorithms\n\n";

    std::cout << "Compile and run:\n";
    std::cout << "  g++ -std=c++17 -O2 -o chapter5 main.cpp\n";
    std::cout << "  ./chapter5\n\n";

    return 0;
}
