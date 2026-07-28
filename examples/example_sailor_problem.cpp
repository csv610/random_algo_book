// =============================================================================
// The Sailor Problem (Example 1.1 — Indicator Variables & Linearity)
// =============================================================================
//
// Problem: n sailors each pick a cabin uniformly at random. How many
// sailors end up in their own cabin on average?
//
// Solution via indicator random variables:
//   Let X_i = 1 if sailor i gets their own cabin, 0 otherwise.
//   Pr[X_i = 1] = 1/n (each cabin is equally likely).
//   By linearity of expectation:
//     E[sum X_i] = sum E[X_i] = n * (1/n) = 1
//
// So the expected number of correctly assigned sailors is ALWAYS 1,
// regardless of n. This is a classic application of indicator variables
// and linearity of expectation — a fundamental technique in randomized
// algorithms and probabilistic analysis.
//
// This example runs a Monte Carlo simulation to empirically verify
// the theoretical result.
// =============================================================================

#include <iostream>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include <iomanip>

namespace ral {

void demonstrate_sailor_problem_monte_carlo(int n_sailors, int num_trials) {
    std::mt19937 rng(42);
    long long total_correct = 0;

    for (int trial = 0; trial < num_trials; trial++) {
        std::vector<int> cabin_assignment(n_sailors);
        std::iota(cabin_assignment.begin(), cabin_assignment.end(), 0);

        // Random permutation = random cabin assignment
        std::shuffle(cabin_assignment.begin(), cabin_assignment.end(), rng);

        int correct = 0;
        for (int i = 0; i < n_sailors; i++) {
            if (cabin_assignment[i] == i) correct++;
        }
        total_correct += correct;
    }

    double empirical_avg = static_cast<double>(total_correct) / num_trials;

    std::cout << "  n = " << n_sailors << " sailors\n";
    std::cout << "  Trials: " << num_trials << "\n";
    std::cout << "  Theoretical E[# correct] = 1.0\n";
    std::cout << "  Empirical average:        " << std::fixed
              << std::setprecision(4) << empirical_avg << "\n\n";
}

} // namespace ral

int main() {
    using namespace ral;

    std::cout << "=== The Sailor Problem (Example 1.1) ===\n\n";

    std::cout << "Problem: n sailors each choose a random cabin.\n";
    std::cout << "Q: How many end up in their own cabin on average?\n\n";

    std::cout << "Proof using indicator variables:\n";
    std::cout << "  Let X_i = 1 if sailor i is in own cabin, 0 otherwise.\n";
    std::cout << "  Pr[X_i = 1] = 1/n\n";
    std::cout << "  E[X_i] = 1/n\n";
    std::cout << "  E[sum X_i] = n * (1/n) = 1  (by linearity)\n\n";

    std::cout << "Monte Carlo verification:\n";
    std::cout << std::string(50, '-') << "\n";

    demonstrate_sailor_problem_monte_carlo(10, 100000);
    demonstrate_sailor_problem_monte_carlo(40, 100000);
    demonstrate_sailor_problem_monte_carlo(100, 100000);
    demonstrate_sailor_problem_monte_carlo(1000, 100000);

    // --- Distribution of the count ---
    std::cout << "Distribution of correct assignments (n=5, 100000 trials):\n";
    {
        int n = 5;
        int trials = 100000;
        std::mt19937 rng(42);
        std::vector<int> hist(n + 1, 0);

        for (int t = 0; t < trials; t++) {
            std::vector<int> perm(n);
            std::iota(perm.begin(), perm.end(), 0);
            std::shuffle(perm.begin(), perm.end(), rng);

            int correct = 0;
            for (int i = 0; i < n; i++)
                if (perm[i] == i) correct++;
            hist[correct]++;
        }

        for (int k = 0; k <= n; k++) {
            std::cout << "  " << k << " correct: "
                      << std::setw(8) << hist[k] << " trials ("
                      << std::fixed << std::setprecision(2)
                      << 100.0 * hist[k] / trials << "%)\n";
        }
    }

    std::cout << "\nDone.\n";
    return 0;
}
