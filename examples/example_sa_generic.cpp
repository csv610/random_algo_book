// =====================================================================
// Generic Simulated Annealing Framework: Maximize a Multimodal Function
//
// Demonstrates the SA framework on a continuous optimization problem:
// finding the global maximum of the multimodal function
//
//   f(x) = sin(x) * exp(-0.1*(x-3)^2) + 0.5*sin(3*x)
//
// on [0, 20]. This function has multiple local maxima; gradient-based
// methods easily get stuck. SA's probabilistic acceptance of worse
// moves allows escaping local optima.
//
// Neighbor: perturb x by a small Gaussian step.
// Cost: -f(x) (minimize negative = maximize).
// =====================================================================

#include "ral/simulated_annealing.h"
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace ral;

double multimodal(double x) {
    return std::sin(x) * std::exp(-0.1 * (x - 3.0) * (x - 3.0))
         + 0.5 * std::sin(3.0 * x);
}

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Generic SA: Multimodal Function Optimization\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    // Encode solution as single-element vector [x]
    // x in [0, 20]
    double xmin = 0.0, xmax = 20.0;

    auto cost_fn = [&](const std::vector<int>& sol) {
        // Map integer representation to continuous x
        double x = xmin + (xmax - xmin) * sol[0] / 10000.0;
        return -multimodal(x);  // minimize negative = maximize
    };

    auto neighbor_fn = [&](const std::vector<int>& sol, std::mt19937& r) {
        std::normal_distribution<double> step(0, 500.0);
        auto result = sol;
        result[0] += static_cast<int>(step(r));
        result[0] = std::clamp(result[0], 0, 10000);
        return result;
    };

    // Grid search for ground truth
    double true_max = -1e18;
    double true_x = 0;
    for (int i = 0; i <= 10000; ++i) {
        double x = xmin + (xmax - xmin) * i / 10000.0;
        double val = multimodal(x);
        if (val > true_max) {
            true_max = val;
            true_x = x;
        }
    }
    std::cout << "Ground truth (grid search): f(" << std::setprecision(4)
              << true_x << ") = " << true_max << "\n\n";

    // Run SA from random starting points
    std::cout << "--- SA from Multiple Starting Points ---\n";
    std::cout << std::setw(8) << "Start"
              << std::setw(12) << "Best x"
              << std::setw(12) << "Best f(x)"
              << std::setw(12) << "Error"
              << "\n";

    double total_error = 0.0;
    int successes = 0;

    for (int trial = 0; trial < 10; ++trial) {
        std::mt19937 trial_rng(1000 + trial);
        int start_x = 1000 + trial_rng() % 8000;  // random start in [0, 20]
        std::vector<int> initial = {start_x};

        auto result = simulated_annealing(
            initial, cost_fn, neighbor_fn,
            50.0,     // initial temp
            0.999,    // cooling rate
            0.001,    // min temp
            50,       // iterations per temp
            trial_rng);

        double best_x = xmin + (xmax - xmin) * result.best_solution[0] / 10000.0;
        double best_val = -result.best_cost;
        double error = std::abs(best_val - true_max);

        if (error < 0.01) successes++;

        std::cout << std::setw(8) << std::setprecision(2) << (xmin + (xmax - xmin) * start_x / 10000.0)
                  << std::setw(12) << std::setprecision(4) << best_x
                  << std::setw(12) << best_val
                  << std::setw(12) << std::setprecision(6) << error
                  << "\n";

        total_error += error;
    }

    std::cout << "\nSuccesses (error < 0.01): " << successes << "/10\n";
    std::cout << "Average absolute error:   " << std::setprecision(6)
              << total_error / 10.0 << "\n";

    // Temperature schedule visualization
    std::cout << "\n--- Temperature Schedule (geometric cooling) ---\n";
    double rate = 0.999;
    std::cout << std::setw(10) << "Step" << std::setw(14) << "Temperature" << "\n";
    for (int step : {0, 100, 500, 1000, 2000, 5000}) {
        double t = 50.0 * std::pow(rate, step);
        std::cout << std::setw(10) << step << std::setw(14) << std::setprecision(6) << t << "\n";
    }

    std::cout << "\nThe generic SA framework accepts any cost and neighbor function,\n";
    std::cout << "making it applicable to combinatorial and continuous optimization.\n";

    return 0;
}
