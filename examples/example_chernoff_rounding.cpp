// example_chernoff_rounding.cpp
// Chernoff-Concentration-Aware Randomized Rounding
//
// Repeats randomized rounding until a feasible solution is found.
// Uses the Chernoff bound: Pr[S > (1+d)*mu] <= exp(-d^2*mu/3)
// to bound the probability that a rounded constraint sum exceeds
// the LP bound. Retries up to a configurable number of trials.
//
// Compile: g++ -std=c++23 -I../include -o example_chernoff_rounding example_chernoff_rounding.cpp

#include <ral/randomized_rounding.h>
#include <ral/linear_programming.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Chernoff-Concentration-Aware Rounding ===\n\n";

    // Weighted Set Cover: same as best_of example
    int n = 4;
    std::vector<double> c = {2.0, 3.0, 1.0, 2.0};

    std::vector<std::vector<double>> A = {
        {-1.0,  0.0,  0.0, -1.0},
        {-1.0, -1.0,  0.0,  0.0},
        { 0.0, -1.0,  0.0,  0.0},
        { 0.0, -1.0, -1.0,  0.0},
        { 0.0,  0.0, -1.0, -1.0},
        { 0.0,  0.0,  0.0, -1.0},
    };
    std::vector<double> b = {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
    std::vector<double> c_neg = {-2.0, -3.0, -1.0, -2.0};

    auto lp = solve_lp(A, b, c_neg, n);

    std::cout << "Set Cover LP relaxation with Chernoff-aware rounding\n\n";

    if (lp.feasible) {
        std::cout << "LP optimum (min cost): " << -lp.objective << "\n\n";

        int feasible_count = 0;
        int best_cost = 1000;
        int trials = 5000;

        for (int t = 0; t < trials; ++t) {
            auto result = chernoff_rounding(lp.x, A, b, c, 100, rng);
            if (result.satisfies_constraints) {
                feasible_count++;
                int cost = 0;
                for (int j = 0; j < n; ++j) cost += static_cast<int>(c[j] * result.rounded[j]);
                if (cost < best_cost) best_cost = cost;
            }
        }

        std::cout << "Chernoff rounding (" << trials << " trials, up to 100 retries each):\n";
        std::cout << "  Feasible: " << 100.0 * feasible_count / trials << "%\n";
        std::cout << "  Best cost found: " << best_cost << "\n\n";

        // Show Chernoff bound for one constraint
        // For constraint: sum of selected sets covering element >= 1
        // mu = LP expectation of the sum
        std::cout << "Chernoff bound illustration:\n";
        std::cout << "  If mu = E[S] for constraint sum, then:\n";
        std::cout << "  Pr[S > (1+d)*mu] <= exp(-d^2 * mu / 3)\n";
        for (double d = 0.5; d <= 2.0; d += 0.5) {
            double bound = std::exp(-d * d * 1.5 / 3.0);  // mu=1.5 example
            std::cout << "  d=" << d << ": Pr[S > " << (1+d) << "*mu] <= "
                      << bound << "\n";
        }
    }

    return 0;
}
