#pragma once

#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>
#include "random_utils.h"
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Simulated Annealing (Kirkpatrick et al., 1983; Cerny, 1985)
// Metaheuristic for combinatorial optimization. Escapes local optima by
// accepting worse solutions with probability exp(-delta_E / T) where T
// is the "temperature" that decreases over time.
//
// Convergence guarantee: under sufficient cooling schedule (T ~ 1/ln(k)),
// converges to global optimum with probability approaching 1.
// ---------------------------------------------------------------------------

struct AnnealingResult {
    std::vector<int> best_solution;
    double best_cost;
    int iterations;
    int improvements;
};

// Generic simulated annealing framework
// cost_fn:        evaluates solution quality (lower is better)
// neighbor_fn:    generates a neighboring solution
// initial_temp:   starting temperature
// cooling_rate:   multiplicative cooling factor (T *= cooling_rate)
// min_temp:       stopping temperature
// max_iter:       maximum iterations per temperature
AnnealingResult simulated_annealing(
    const std::vector<int>& initial,
    std::function<double(const std::vector<int>&)> cost_fn,
    std::function<std::vector<int>(const std::vector<int>&, std::mt19937&)> neighbor_fn,
    double initial_temp,
    double cooling_rate,
    double min_temp,
    int max_iter_per_temp,
    std::mt19937& rng) {

    std::uniform_real_distribution<double> unif(0.0, 1.0);

    auto current = initial;
    double current_cost = cost_fn(current);

    auto best = current;
    double best_cost = current_cost;

    int total_iter = 0;
    int improvements = 0;
    double T = initial_temp;

    while (T > min_temp) {
        for (int i = 0; i < max_iter_per_temp; ++i) {
            auto neighbor = neighbor_fn(current, rng);
            double neighbor_cost = cost_fn(neighbor);
            double delta = neighbor_cost - current_cost;

            if (delta < 0 || unif(rng) < std::exp(-delta / T)) {
                current = neighbor;
                current_cost = neighbor_cost;
                total_iter++;

                if (current_cost < best_cost) {
                    best = current;
                    best_cost = current_cost;
                    improvements++;
                }
            }
        }
        T *= cooling_rate;
    }

    return {best, best_cost, total_iter, improvements};
}

// ---------------------------------------------------------------------------
// TSP Example: Traveling Salesman Problem
// ---------------------------------------------------------------------------

struct TSPInstance {
    std::vector<double> x, y;
};

inline double tsp_cost(const std::vector<int>& tour, const TSPInstance& inst) {
    double cost = 0.0;
    int n = static_cast<int>(tour.size());
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        double dx = inst.x[tour[i]] - inst.x[tour[j]];
        double dy = inst.y[tour[i]] - inst.y[tour[j]];
        cost += std::sqrt(dx * dx + dy * dy);
    }
    return cost;
}

// 2-opt swap: reverse a segment of the tour
inline std::vector<int> tsp_neighbor_2opt(
    const std::vector<int>& tour, std::mt19937& rng) {

    int n = static_cast<int>(tour.size());
    auto result = tour;

    std::uniform_int_distribution<int> dist(0, n - 1);
    int i = dist(rng);
    int j = dist(rng);
    if (i > j) std::swap(i, j);
    if (i == j || (i == 0 && j == n - 1)) return result;

    std::reverse(result.begin() + i, result.begin() + j + 1);
    return result;
}

// Or-opt: move a segment of 1-3 cities to a new position
inline std::vector<int> tsp_neighbor_or_opt(
    const std::vector<int>& tour, std::mt19937& rng) {

    int n = static_cast<int>(tour.size());
    auto result = tour;

    std::uniform_int_distribution<int> dist_len(1, 3);
    std::uniform_int_distribution<int> dist_pos(0, n - 1);

    int len = dist_len(rng);
    int from = dist_pos(rng);
    int to = dist_pos(rng);

    if (from + len > n) from = n - len;
    if (to >= from && to <= from + len) return result;

    std::vector<int> segment(result.begin() + from, result.begin() + from + len);
    result.erase(result.begin() + from, result.begin() + from + len);
    result.insert(result.begin() + to, segment.begin(), segment.end());
    return result;
}

// ---------------------------------------------------------------------------
// Knapsack Example
// ---------------------------------------------------------------------------

struct KnapsackInstance {
    std::vector<int> weights;
    std::vector<int> values;
    int capacity;
};

inline double knapsack_cost(const std::vector<int>& solution, const KnapsackInstance& inst) {
    int total_weight = 0;
    int total_value = 0;
    int n = static_cast<int>(solution.size());
    for (int i = 0; i < n; ++i) {
        if (solution[i]) {
            total_weight += inst.weights[i];
            total_value += inst.values[i];
        }
    }
    // Heavy penalty for exceeding capacity
    if (total_weight > inst.capacity) {
        return 1e9 + (total_weight - inst.capacity);
    }
    return -total_value;  // minimize negative value = maximize value
}

inline std::vector<int> knapsack_neighbor(
    const std::vector<int>& solution, std::mt19937& rng) {

    auto result = solution;
    int n = static_cast<int>(result.size());
    std::uniform_int_distribution<int> dist(0, n - 1);
    int i = dist(rng);
    result[i] = 1 - result[i];  // flip one item
    return result;
}

// ---------------------------------------------------------------------------
// Demonstration
// ---------------------------------------------------------------------------
inline void demonstrate_simulated_annealing() {
    std::mt19937 rng(42);

    println("=== Simulated Annealing ===\n");

    // --- TSP ---
    println("--- 1. Traveling Salesman Problem (50 cities) ---");
    {
        int n = 50;
        TSPInstance inst;
        inst.x.resize(n);
        inst.y.resize(n);
        std::uniform_real_distribution<double> coord(0.0, 100.0);
        for (int i = 0; i < n; ++i) {
            inst.x[i] = coord(rng);
            inst.y[i] = coord(rng);
        }

        // Initial tour: identity
        std::vector<int> initial(n);
        std::iota(initial.begin(), initial.end(), 0);

        double initial_cost = tsp_cost(initial, inst);
        println("  Initial tour cost: {:.2f}", initial_cost);

        auto result = simulated_annealing(
            initial,
            [&](const std::vector<int>& t) { return tsp_cost(t, inst); },
            tsp_neighbor_2opt,
            1000.0,   // initial temp
            0.995,    // cooling rate
            0.01,     // min temp
            100,      // iterations per temp
            rng);

        println("  After simulated annealing:");
        println("    Best cost: {:.2f}", result.best_cost);
        println("    Improvement: {:.1f}%", 100.0 * (1.0 - result.best_cost / initial_cost));
        println("    Total iterations: {}", result.iterations);
        println("    Improvements found: {}", result.improvements);

        // Nearest neighbor heuristic for comparison
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
        println("  Nearest neighbor cost: {:.2f}", nn_cost);
        println("  SA vs NN improvement: {:.1f}%", 100.0 * (1.0 - result.best_cost / nn_cost));
    }

    // --- Knapsack ---
    println("\n--- 2. 0/1 Knapsack (100 items) ---");
    {
        int n = 100;
        KnapsackInstance inst;
        inst.capacity = 500;
        inst.weights.resize(n);
        inst.values.resize(n);
        std::uniform_int_distribution<int> wdist(1, 20);
        std::uniform_int_distribution<int> vdist(1, 100);
        for (int i = 0; i < n; ++i) {
            inst.weights[i] = wdist(rng);
            inst.values[i] = vdist(rng);
        }

        // Initial solution: all zeros
        std::vector<int> initial(n, 0);

        println("  Items: {}, Capacity: {}", n, inst.capacity);

        auto result = simulated_annealing(
            initial,
            [&](const std::vector<int>& s) { return knapsack_cost(s, inst); },
            knapsack_neighbor,
            100.0,
            0.999,
            0.001,
            200,
            rng);

        int total_weight = 0, total_value = 0;
        for (int i = 0; i < n; ++i) {
            if (result.best_solution[i]) {
                total_weight += inst.weights[i];
                total_value += inst.values[i];
            }
        }
        println("  Best value: {}", total_value);
        println("  Total weight: {}/{}", total_weight, inst.capacity);
        println("  Items selected: {}",
                      std::accumulate(result.best_solution.begin(), result.best_solution.end(), 0));
        println("  Iterations: {}", result.iterations);
    }
}

} // namespace ral
