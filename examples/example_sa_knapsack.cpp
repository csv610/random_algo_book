// =====================================================================
// Simulated Annealing for the 0/1 Knapsack Problem
//
// Given n items with weights w_i and values v_i, select a subset that
// maximizes total value without exceeding capacity C.
//
// Neighborhood: flip one item in/out of the knapsack.
// Cost function: -total_value with a heavy penalty for exceeding capacity.
// SA accepts worse solutions early to explore the solution space, then
// converges as temperature cools.
//
// This example solves a 100-item knapsack instance and reports the
// best value found, weight utilization, and items selected.
// =====================================================================

#include "ral/simulated_annealing.h"
#include <iostream>
#include <random>
#include <vector>
#include <numeric>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Simulated Annealing for 0/1 Knapsack\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    int n = 100;
    int capacity = 500;

    // Generate random items
    KnapsackInstance inst;
    inst.capacity = capacity;
    inst.weights.resize(n);
    inst.values.resize(n);
    std::uniform_int_distribution<int> wdist(1, 20);
    std::uniform_int_distribution<int> vdist(1, 100);
    for (int i = 0; i < n; ++i) {
        inst.weights[i] = wdist(rng);
        inst.values[i] = vdist(rng);
    }

    std::cout << "Items: " << n << ", Capacity: " << capacity << "\n\n";

    // Greedy baseline: value/weight ratio
    std::vector<int> greedy(n, 0);
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return (double)inst.values[a] / inst.weights[a] >
               (double)inst.values[b] / inst.weights[b];
    });
    int greedy_weight = 0, greedy_value = 0;
    for (int i : order) {
        if (greedy_weight + inst.weights[i] <= capacity) {
            greedy[i] = 1;
            greedy_weight += inst.weights[i];
            greedy_value += inst.values[i];
        }
    }
    std::cout << "Greedy baseline value: " << greedy_value
              << " (weight: " << greedy_weight << "/" << capacity << ")\n";

    // SA: start from all-zeros
    std::vector<int> initial(n, 0);
    auto result = simulated_annealing(
        initial,
        [&](const std::vector<int>& s) { return knapsack_cost(s, inst); },
        knapsack_neighbor,
        100.0,     // initial temperature
        0.999,     // cooling rate (slow)
        0.001,     // min temperature
        200,       // iterations per temp
        rng);

    // Analyze result
    int total_weight = 0, total_value = 0;
    int items_selected = 0;
    for (int i = 0; i < n; ++i) {
        if (result.best_solution[i]) {
            total_weight += inst.weights[i];
            total_value += inst.values[i];
            items_selected++;
        }
    }

    std::cout << "\n--- SA Results ---\n";
    std::cout << "Best value found:    " << total_value << "\n";
    std::cout << "Total weight:        " << total_weight << "/" << capacity << "\n";
    std::cout << "Items selected:      " << items_selected << "/" << n << "\n";
    std::cout << "Weight utilization:  "
              << std::fixed << std::setprecision(1)
              << 100.0 * total_weight / capacity << "%\n";
    std::cout << "Value improvement over greedy: "
              << std::setprecision(1)
              << 100.0 * (total_value - greedy_value) / greedy_value << "%\n";
    std::cout << "Total accepted moves: " << result.iterations << "\n";
    std::cout << "Best-improving moves: " << result.improvements << "\n";

    // Show selected items
    std::cout << "\n--- Selected Items (first 20) ---\n";
    std::cout << std::setw(6) << "Item" << std::setw(8) << "Weight"
              << std::setw(8) << "Value" << std::setw(8) << "Ratio" << "\n";
    int shown = 0;
    for (int i = 0; i < n && shown < 20; ++i) {
        if (result.best_solution[i]) {
            std::cout << std::setw(6) << i
                      << std::setw(8) << inst.weights[i]
                      << std::setw(8) << inst.values[i]
                      << std::setw(8) << std::setprecision(2)
                      << (double)inst.values[i] / inst.weights[i]
                      << "\n";
            shown++;
        }
    }

    std::cout << "\nSA navigates the 2^100 solution space by accepting\n";
    std::cout << "exploratory moves at high temperature, then refining\n";
    std::cout << "as the system cools toward the optimal solution.\n";

    return 0;
}
