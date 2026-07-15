#include <iostream>
#include <vector>
#include <cmath>
#include "random_utils.h"
#include "min_cut.h"
#include "las_vegas_monte_carlo.h"
#include "binary_planar_partition.h"
#include "probabilistic_recurrence.h"

using namespace chapter1;

void print_header() {
    std::cout << "+==============================================================+\n";
    std::cout << "|        RANDOMIZED ALGORITHMS - CHAPTER 1 IMPLEMENTATION    |\n";
    std::cout << "+==============================================================+\n\n";
}

void print_section(int num, const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Section 1." << num << ": " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

int main() {
    print_header();
    
    // Section 1.1: Min-Cut Algorithm
    print_section(1, "A Min-Cut Algorithm");
    demonstrate_min_cut();
    
    // Section 1.2: Las Vegas and Monte Carlo
    print_section(2, "Las Vegas and Monte Carlo");
    demonstrate_las_vegas();
    demonstrate_monte_carlo();
    demonstrate_conversion();
    
    // Section 1.3: Binary Planar Partitions
    print_section(3, "Binary Planar Partitions");
    demonstrate_binary_planar_partitions();
    demonstrate_sailor_problem();
    
    // Section 1.4: Probabilistic Recurrence
    print_section(4, "A Probabilistic Recurrence");
    demonstrate_probabilistic_recurrence();
    demonstrate_geometric_distribution();
    analyze_recursion_depth();
    
    // Section 1.5: Complexity Classes (theoretical, no code demo needed)
    print_section(5, "Computation Model and Complexity Classes");
    std::cout << "This section covers theoretical concepts:\n";
    std::cout << "- RAM vs Turing Machine models\n";
    std::cout << "- Complexity classes: P, NP, RP, co-RP, ZPP, BPP, PP\n";
    std::cout << "- Polynomial reductions and NP-completeness\n\n";
    
    std::cout << "Key relationships:\n";
    std::cout << "P <= RP <= NP <= PSPACE <= EXP <= NEXP\n";
    std::cout << "RP <= BPP <= PP\n";
    std::cout << "ZPP = RP & co-RP\n\n";
    
    // Summary
    std::cout << std::string(60, '=') << "\n";
    std::cout << "SUMMARY OF CHAPTER 1\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    std::cout << "Key Concepts Demonstrated:\n\n";
    
    std::cout << "1. RANDOMIZED MIN-CUT (Karger's Algorithm)\n";
    std::cout << "   - Randomized algorithm for minimum cut\n";
    std::cout << "   - Probability of success: >= 2/n^2 per trial\n";
    std::cout << "   - After n^2/2 trials, failure probability < 1/e\n";
    std::cout << "   - Deterministic alternatives require network flow techniques\n\n";
    
    std::cout << "2. LAS VEGAS vs MONTE CARLO\n";
    std::cout << "   - Las Vegas: Always correct, running time is random\n";
    std::cout << "   - Monte Carlo: May be incorrect, but error probability is bounded\n";
    std::cout << "   - Can convert Monte Carlo to Las Vegas using verification\n";
    std::cout << "   - Geometric distribution: expected attempts = 1/p\n\n";
    
    std::cout << "3. BINARY PLANAR PARTITIONS\n";
    std::cout << "   - Applications in computer graphics (hidden line elimination)\n";
    std::cout << "   - RandAuto algorithm: expected size O(n log n)\n";
    std::cout << "   - Uses linearity of expectation (no independence required)\n";
    std::cout << "   - Probabilistic method: proves existence of a partition with O(n log n) expected size\n\n";
    
    std::cout << "4. PROBABILISTIC RECURRENCE\n";
    std::cout << "   - Find algorithm: expected O(n) time\n";
    std::cout << "   - Theorem 1.3: bounds expected recursion depth\n";
    std::cout << "   - Connection to geometric distribution\n";
    std::cout << "   - General technique for analyzing randomized algorithms\n\n";
    
    std::cout << "5. COMPLEXITY CLASSES\n";
    std::cout << "   - P: deterministic polynomial time\n";
    std::cout << "   - NP: nondeterministic polynomial time\n";
    std::cout << "   - RP: randomized polynomial time (one-sided error)\n";
    std::cout << "   - BPP: bounded-error probabilistic polynomial time\n";
    std::cout << "   - ZPP: zero-error probabilistic polynomial time (Las Vegas)\n\n";
    
    std::cout << "Compile and run:\n";
    std::cout << "  g++ -std=c++17 -O2 -o chapter1 main.cpp\n";
    std::cout << "  ./chapter1\n\n";
    
    return 0;
}