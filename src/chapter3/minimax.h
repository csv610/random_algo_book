#pragma once
// Minimax Principle and Yao's Technique
// Demonstrates: scissors-paper-stone game, mixed strategies, minimax theorem

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>

namespace chapter3 {

// Payoff matrix for scissors-paper-stone
// Rows: Scissors(0), Paper(1), Stone(2)
// Columns: Scissors(0), Paper(1), Stone(2)
// Entry M[i][j] = payoff from row player's perspective
const std::vector<std::vector<int>> SPS_PAYOFF = {
    { 0,  1, -1},  // Scissors vs S,P,St
    {-1,  0,  1},  // Paper vs S,P,St
    { 1, -1,  0}   // Stone vs S,P,St
};

// Compute max_i min_j M[i][j] (row player's security level)
double maximin(const std::vector<std::vector<double>>& M) {
    double result = -1e9;
    for (size_t i = 0; i < M.size(); ++i) {
        double row_min = 1e9;
        for (size_t j = 0; j < M[i].size(); ++j)
            row_min = std::min(row_min, M[i][j]);
        result = std::max(result, row_min);
    }
    return result;
}

// Compute min_j max_i M[i][j] (column player's security level)
double minimax(const std::vector<std::vector<double>>& M) {
    double result = 1e9;
    for (size_t j = 0; j < M[0].size(); ++j) {
        double col_max = -1e9;
        for (size_t i = 0; i < M.size(); ++i)
            col_max = std::max(col_max, M[i][j]);
        result = std::min(result, col_max);
    }
    return result;
}

// Solve a 2-player zero-sum game using linear programming on the simplex
// For a 3x3 matrix, enumerate extreme points
struct MixedStrategy {
    std::vector<double> prob;
};

MixedStrategy find_optimal_row(const std::vector<std::vector<double>>& M,
                               int trials = 100000,
                               unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::discrete_distribution<int> dist({1, 1, 1});

    MixedStrategy best;
    best.prob = {1.0/3, 1.0/3, 1.0/3};
    double best_val = 1e9;

    for (int t = 0; t < trials; ++t) {
        // Generate random mixed strategy on simplex
        std::vector<double> p(3);
        for (int i = 0; i < 3; ++i)
            p[i] = std::exponential_distribution<>(1.0)(rng);
        double sum = p[0] + p[1] + p[2];
        for (int i = 0; i < 3; ++i) p[i] /= sum;

        // Expected payoff against worst-case pure strategy
        double worst = 1e9;
        for (int j = 0; j < 3; ++j) {
            double ev = 0;
            for (int i = 0; i < 3; ++i)
                ev += p[i] * M[i][j];
            worst = std::min(worst, ev);
        }
        if (worst > best_val - 1e-9) {
            best_val = worst;
            best.prob = p;
        }
    }
    return best;
}

void demonstrate_minimax() {
    std::cout << "Minimax Principle (Scissors-Paper-Stone)\n";
    std::cout << "Payoff Matrix:\n";
    std::cout << "           Scissors  Paper  Stone\n";
    const char* names[] = {"Scissors", "Paper  ", "Stone  "};
    for (int i = 0; i < 3; ++i) {
        std::cout << "  " << names[i] << "  ";
        for (int j = 0; j < 3; ++j)
            std::cout << std::setw(8) << SPS_PAYOFF[i][j];
        std::cout << "\n";
    }
    std::cout << "\n";

    std::vector<std::vector<double>> M(3, std::vector<double>(3));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            M[i][j] = SPS_PAYOFF[i][j];

    double vm = maximin(M);
    double vM = minimax(M);

    std::cout << "Pure strategies:\n";
    std::cout << "  max_i min_j M[i][j] = " << vm << "\n";
    std::cout << "  min_j max_i M[i][j] = " << vM << "\n";
    std::cout << "  Saddle point exists? " << (vm == vM ? "Yes" : "No") << "\n\n";

    // Optimal mixed strategy (uniform by symmetry)
    MixedStrategy opt = find_optimal_row(M);
    std::cout << "Optimal mixed strategy (uniform by symmetry):\n";
    std::cout << "  p = (" << opt.prob[0] << ", "
              << opt.prob[1] << ", " << opt.prob[2] << ")\n";

    double game_value = 0;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            game_value += opt.prob[i] * M[i][j] * opt.prob[j];
    std::cout << "  Game value V = " << game_value
              << " (should be 0 for this game)\n\n";

    // Yao's Minimax Principle demonstration
    std::cout << "Yao's Minimax Principle:\n";
    std::cout << "  For any input distribution D on instances:\n";
    std::cout << "    E_{x~D}[T_A(x)] >= min_{A deterministic} E_{x~D}[T_A(x)]\n";
    std::cout << "  for any randomized algorithm A.\n";
    std::cout << "  This provides a lower bound on the expected cost of any randomized algorithm.\n\n";
}

}  // namespace chapter3
