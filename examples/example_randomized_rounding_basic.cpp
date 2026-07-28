// example_randomized_rounding_basic.cpp
// Basic Randomized Rounding of LP Solution
//
// Given a fractional LP solution x* in [0,1]^n, round each coordinate
// independently: x_i = 1 with probability x*_i, else 0. This produces
// an integer solution whose expected cost equals the LP optimum.
//
// Compile: g++ -std=c++23 -I../include -o example_randomized_rounding_basic example_randomized_rounding_basic.cpp

#include <ral/randomized_rounding.h>
#include <ral/linear_programming.h>
#include <iostream>
#include <vector>
#include <random>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Basic Randomized Rounding ===\n\n";

    // Simple LP: Maximize 3x + 5y subject to: x + 2y <= 4, 0 <= x,y <= 1
    std::vector<std::vector<double>> A = {
        {1.0, 2.0},    // x + 2y <= 4
        {-1.0, 0.0},   // x >= 0
        {0.0, -1.0}    // y >= 0
    };
    std::vector<double> b = {4.0, 0.0, 0.0};
    std::vector<double> c = {-3.0, -5.0};  // negate for minimization

    auto lp = solve_lp(A, b, c, 2);

    std::cout << "Maximize 3x + 5y s.t. x + 2y <= 4, 0 <= x,y <= 1\n\n";

    if (lp.feasible) {
        std::cout << "LP optimum: x=" << -lp.x[0] << ", y=" << -lp.x[1]
                  << ", objective=" << -lp.objective << "\n\n";

        // Run basic rounding many times
        int feasible_count = 0;
        int best_obj = 0;
        int trials = 10000;

        for (int t = 0; t < trials; ++t) {
            auto result = randomized_rounding_basic(lp.x, A, b, c, rng);
            if (result.satisfies_constraints) {
                feasible_count++;
                if (result.rounded_objective > best_obj) {
                    best_obj = result.rounded_objective;
                }
            }
        }

        std::cout << "Rounding results (" << trials << " trials):\n";
        std::cout << "  Fraction feasible: " << 100.0 * feasible_count / trials << "%\n";
        std::cout << "  Best integer objective: " << best_obj << "\n";

        // Enumerate all integer solutions
        std::cout << "\nAll feasible integer solutions:\n";
        for (int x = 0; x <= 1; ++x) {
            for (int y = 0; y <= 1; ++y) {
                if (x + 2 * y <= 4) {
                    std::cout << "  (" << x << ", " << y << "): objective = "
                              << 3 * x + 5 * y << "\n";
                }
            }
        }
    }

    return 0;
}
