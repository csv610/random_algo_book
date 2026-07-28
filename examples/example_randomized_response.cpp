// =====================================================================
// Randomized Response for Privacy-Preserving Surveys (Warner, 1965)
//
// Each respondent answers truthfully with probability p, and randomly
// (flips a fair coin) otherwise. This provides plausible deniability:
// the surveyor cannot determine any individual's true answer.
//
// From aggregated responses, the true population proportion can be
// recovered via: true_p = (q - (1-p)) / (2p - 1)
// where q = fraction of "yes" responses.
//
// This example surveys a sensitive question (e.g., "Have you ever
// cheated on a tax return?") where respondents would not answer
// truthfully without anonymity guarantees.
// =====================================================================

#include "ral/differential_privacy.h"
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Randomized Response: Privacy-Preserving Survey\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    int n = 10000;
    double true_proportion = 0.30;  // 30% truly have the attribute
    double p = 0.75;                // answer truthfully 75% of the time

    std::cout << "Survey setup:\n";
    std::cout << "  Respondents:           " << n << "\n";
    std::cout << "  True 'yes' proportion: " << true_proportion * 100 << "%\n";
    std::cout << "  Truthfulness prob p:   " << p << "\n";
    std::cout << "  Flip probability:      " << (1.0 - p) << "\n\n";

    // Generate true answers
    std::vector<bool> true_answers(n);
    std::uniform_real_distribution<double> unif(0.0, 1.0);
    int true_yes = 0;
    for (int i = 0; i < n; ++i) {
        true_answers[i] = (unif(rng) < true_proportion);
        if (true_answers[i]) true_yes++;
    }
    std::cout << "True 'yes' count: " << true_yes << "/" << n << "\n\n";

    // Collect randomized responses
    auto result = collect_randomized_responses(true_answers, p, rng);

    int reported_yes = 0;
    for (bool r : result.raw_responses) reported_yes += r;

    std::cout << "--- Raw Responses ---\n";
    std::cout << "Reported 'yes': " << reported_yes << "/" << n << "\n";
    std::cout << "Fraction 'yes': " << std::fixed << std::setprecision(4)
              << (double)reported_yes / n << "\n";
    std::cout << "Truthful answers given: " << result.truthful_count << "/"
              << n << " (" << std::setprecision(1)
              << 100.0 * result.truthful_count / n << "%)\n\n";

    // Recover true proportion
    double estimated = estimate_proportion(result.raw_responses, p);
    std::cout << "--- Recovery ---\n";
    std::cout << "Estimated true proportion: " << std::setprecision(4)
              << estimated << "\n";
    std::cout << "True proportion:           " << true_proportion << "\n";
    std::cout << "Absolute error:            " << std::abs(estimated - true_proportion) << "\n";

    // Show why privacy holds
    std::cout << "\n--- Privacy Guarantee ---\n";
    std::cout << "For any individual, the server sees:\n";
    std::cout << "  P(reported=yes | true=yes) = " << p << "\n";
    std::cout << "  P(reported=yes | true=no)  = " << (1.0 - p) << "\n";
    std::cout << "Since these are equal when p=0.5, and close for p>0.5,\n";
    std::cout << "the server cannot reliably infer any individual's truth.\n";

    // Run multiple trials to show estimation accuracy
    std::cout << "\n--- Estimation Accuracy Over 10 Trials ---\n";
    std::cout << std::setw(8) << "Trial"
              << std::setw(14) << "Estimated"
              << std::setw(12) << "Error"
              << "\n";

    for (int trial = 0; trial < 10; ++trial) {
        std::mt19937 trial_rng(500 + trial);
        std::vector<bool> trial_answers(n);
        for (int i = 0; i < n; ++i) {
            trial_answers[i] = (unif(trial_rng) < true_proportion);
        }
        auto trial_result = collect_randomized_responses(trial_answers, p, trial_rng);
        double est = estimate_proportion(trial_result.raw_responses, p);
        std::cout << std::setw(8) << trial + 1
                  << std::setw(14) << std::setprecision(4) << est
                  << std::setw(12) << std::setprecision(4) << std::abs(est - true_proportion)
                  << "\n";
    }

    std::cout << "\nRandomized response provides local differential privacy:\n";
    std::cout << "each respondent's true answer is plausibly deniable.\n";

    return 0;
}
