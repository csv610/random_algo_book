// =====================================================================
// Simulated Annealing for the Traveling Salesman Problem (TSP)
//
// Uses the generic SA framework with a 2-opt neighborhood to find
// near-optimal tours. The 2-opt move reverses a segment of the tour,
// effectively uncrossing edges.
//
// SA accepts worse solutions with probability exp(-delta/T) where T
// decreases over time (geometric cooling: T *= 0.995). This allows
// escaping local optima. Under sufficient cooling, SA converges to
// the global optimum with probability approaching 1.
//
// This example solves TSP for 50 randomly placed cities and compares
// against nearest-neighbor and random baseline tours.
// =====================================================================

#include "ral/simulated_annealing.h"
#include <iostream>
#include <random>
#include <vector>
#include <numeric>
#include <cmath>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Simulated Annealing for TSP (2-opt)\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    int n = 50;
    std::cout << "Cities: " << n << "\n\n";

    // Generate random city coordinates
    TSPInstance inst;
    inst.x.resize(n);
    inst.y.resize(n);
    std::uniform_real_distribution<double> coord(0.0, 100.0);
    for (int i = 0; i < n; ++i) {
        inst.x[i] = coord(rng);
        inst.y[i] = coord(rng);
    }

    // Random baseline: many random tours
    double random_best = 1e18;
    for (int trial = 0; trial < 1000; ++trial) {
        std::vector<int> tour(n);
        std::iota(tour.begin(), tour.end(), 0);
        std::shuffle(tour.begin(), tour.end(), rng);
        random_best = std::min(random_best, tsp_cost(tour, inst));
    }
    std::cout << "Random tour (best of 1000): " << std::fixed
              << std::setprecision(2) << random_best << "\n";

    // Nearest-neighbor heuristic
    std::vector<int> nn_tour;
    std::vector<bool> visited(n, false);
    nn_tour.push_back(0);
    visited[0] = true;
    for (int step = 1; step < n; ++step) {
        int last = nn_tour.back();
        int best_next = -1;
        double best_dist = 1e18;
        for (int j = 0; j < n; ++j) {
            if (visited[j]) continue;
            double dx = inst.x[last] - inst.x[j];
            double dy = inst.y[last] - inst.y[j];
            double d = std::sqrt(dx * dx + dy * dy);
            if (d < best_dist) {
                best_dist = d;
                best_next = j;
            }
        }
        nn_tour.push_back(best_next);
        visited[best_next] = true;
    }
    double nn_cost = tsp_cost(nn_tour, inst);
    std::cout << "Nearest-neighbor tour:     " << nn_cost << "\n";

    // Simulated annealing with 2-opt
    std::vector<int> initial(n);
    std::iota(initial.begin(), initial.end(), 0);
    double initial_cost = tsp_cost(initial, inst);
    std::cout << "Identity tour (initial):  " << initial_cost << "\n\n";

    auto result = simulated_annealing(
        initial,
        [&](const std::vector<int>& t) { return tsp_cost(t, inst); },
        tsp_neighbor_2opt,
        1000.0,   // initial temperature
        0.995,    // cooling rate
        0.01,     // min temperature
        100,      // iterations per temperature
        rng);

    std::cout << "--- SA Results ---\n";
    std::cout << "Best cost found:    " << result.best_cost << "\n";
    std::cout << "Improvement vs identity: "
              << std::setprecision(1)
              << 100.0 * (1.0 - result.best_cost / initial_cost) << "%\n";
    std::cout << "Improvement vs NN:  "
              << 100.0 * (1.0 - result.best_cost / nn_cost) << "%\n";
    std::cout << "Improvement vs random: "
              << 100.0 * (1.0 - result.best_cost / random_best) << "%\n";
    std::cout << "Total accepted moves: " << result.iterations << "\n";
    std::cout << "Best-improving moves: " << result.improvements << "\n";

    // Print the tour order (first and last 10 cities)
    std::cout << "\n--- Tour Order (first/last 10) ---\n";
    std::cout << "Start: ";
    for (int i = 0; i < 10; ++i)
        std::cout << result.best_solution[i] << " ";
    std::cout << "...\n";
    std::cout << "End:   ... ";
    for (int i = n - 10; i < n; ++i)
        std::cout << result.best_solution[i] << " ";
    std::cout << "\n";

    std::cout << "\nSimulated annealing with 2-opt finds near-optimal TSP tours\n";
    std::cout << "by accepting worse moves early (high T) and narrowing to\n";
    std::cout << "improvements only as T -> 0 (geometric cooling schedule).\n";

    return 0;
}
