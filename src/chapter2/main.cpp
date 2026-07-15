#include <iostream>
#include "game_tree.h"
#include "minimax.h"
#include "adleman.h"

using namespace chapter2;

void print_header() {
    std::cout << "+==============================================================+\n";
    std::cout << "|        RANDOMIZED ALGORITHMS - CHAPTER 2 IMPLEMENTATION    |\n";
    std::cout << "|        Based on Motwani & Raghavan's Textbook              |\n";
    std::cout << "+==============================================================+\n\n";
}

void print_section(int num, const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Section 2." << num << ": " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

int main() {
    print_header();

    // Section 2.1: Game Tree Evaluation
    print_section(1, "Game Tree Evaluation");
    demonstrate_game_tree();

    // Section 2.2: The Minimax Principle
    print_section(2, "The Minimax Principle");
    demonstrate_minimax();

    // Section 2.3: Randomness and Non-uniformity
    print_section(3, "Randomness and Non-uniformity");
    demonstrate_adleman();

    // Summary
    std::cout << std::string(60, '=') << "\n";
    std::cout << "SUMMARY OF CHAPTER 2\n";
    std::cout << std::string(60, '=') << "\n\n";

    std::cout << "Key Concepts Demonstrated:\n\n";

    std::cout << "1. GAME TREE EVALUATION\n";
    std::cout << "   - Randomized algorithm: O(n^0.793) expected\n";
    std::cout << "   - Deterministic lower bound: O(n) worst case\n";
    std::cout << "   - Las Vegas algorithm: always correct\n";
    std::cout << "   - Key idea: random order foils adversary\n\n";

    std::cout << "2. MINIMAX PRINCIPLE\n";
    std::cout << "   - Two-person zero-sum games\n";
    std::cout << "   - Pure strategies: max_i min_j M[i][j] <= min_j max_i M[i][j]\n";
    std::cout << "   - Mixed strategies: Von Neumann's Minimax Theorem\n";
    std::cout << "   - Yao's Principle: lower bounds on randomized algorithms\n\n";

    std::cout << "3. ADLEMAN'S THEOREM\n";
    std::cout << "   - BPP <= P/poly\n";
    std::cout << "   - Non-uniform advice: polynomial-size for each input length\n";
    std::cout << "   - Advice depends on |x|, not x itself\n";
    std::cout << "   - Separates BPP from non-uniform classes\n\n";

    std::cout << "Compile and run:\n";
    std::cout << "  g++ -std=c++17 -O2 -o chapter2 main.cpp\n";
    std::cout << "  ./chapter2\n\n";

    return 0;
}
