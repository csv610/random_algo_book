// example_best_of_rounding.cpp
// Best-of-N Randomized Rounding
//
// Repeats basic randomized rounding N times and keeps the best result
// (prefers feasible solutions, then highest objective). This increases
// the chance of finding a good integer solution.
//
// Compile: g++ -std=c++23 -I../include -o example_best_of_rounding example_best_of_rounding.cpp

#include <ral/randomized_rounding.h>
#include <ral/linear_programming.h>
#include <iostream>
#include <vector>
#include <random>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Best-of-N Randomized Rounding ===\n\n";

    // Set Cover LP relaxation:
    // 6 elements, 4 sets with costs {2, 3, 1, 2}
    // Minimize cost s.t. every element covered
    int n = 4;
    std::vector<double> c = {2.0, 3.0, 1.0, 2.0};

    // Coverage constraints (as <= for LP solver)
    std::vector<std::vector<double>> A = {
        {-1.0,  0.0,  0.0, -1.0},  // element 0: sets 0, 3
        {-1.0, -1.0,  0.0,  0.0},  // element 1: sets 0, 1
        { 0.0, -1.0,  0.0,  0.0},  // element 2: set 1
        { 0.0, -1.0, -1.0,  0.0},  // element 3: sets 1, 2
        { 0.0,  0.0, -1.0, -1.0},  // element 4: sets 2, 3
        { 0.0,  0.0,  0.0, -1.0},  // element 5: set 3
    };
    std::vector<double> b = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
    std::vector<double> c_neg = {-2.0, -3.0, -1.0, -2.0};

    auto lp = solve_lp(A, b, c_neg, n);

    std::cout << "Weighted Set Cover: minimize cost to cover all elements\n";
    std::cout << "Sets: S0={0,1}(cost=2), S1={1,2,3}(cost=3), S2={3,4}(cost=1), S3={0,4,5}(cost=2)\n\n";

    if (lp.feasible) {
        std::cout << "LP optimum (min cost): " << -lp.objective << "\n";
        std::cout << "LP solution: (";
        for (int j = 0; j < n; ++j) {
            if (j) std::cout << ", ";
            std::cout << -lp.x[j];
        }
        std::cout << ")\n\n";

        // Compare basic vs best-of-N
        int basic_feasible = 0, basic_best = 1000;
        int best_of_feasible = 0, best_of_best = 1000;
        int trials = 1000;

        for (int t = 0; t < trials; ++t) {
            auto basic = randomized_rounding_basic(lp.x, A, b, c, rng);
            if (basic.satisfies_constraints) {
                basic_feasible++;
                int cost = 0;
                for (int j = 0; j < n; ++j) cost += static_cast<int>(c[j] * basic.rounded[j]);
                if (cost < basic_best) basic_best = cost;
            }

            auto best = randomized_rounding_best_of(lp.x, A, b, c, 20, rng);
            if (best.satisfies_constraints) {
                best_of_feasible++;
                int cost = 0;
                for (int j = 0; j < n; ++j) cost += static_cast<int>(c[j] * best.rounded[j]);
                if (cost < best_of_best) best_of_best = cost;
            }
        }

        std::cout << "Basic rounding (" << trials << " trials):\n";
        std::cout << "  Feasible: " << 100.0 * basic_feasible / trials << "%\n";
        std::cout << "  Best cost found: " << basic_best << "\n\n";

        std::cout << "Best-of-20 rounding (" << trials << " trials):\n";
        std::cout << "  Feasible: " << 100.0 * best_of_feasible / trials << "%\n";
        std::cout << "  Best cost found: " << best_of_best << "\n";
    }

    return 0;
}
