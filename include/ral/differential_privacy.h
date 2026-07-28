#pragma once

#include <vector>
#include <random>
#include <cmath>
#include <functional>
#include <iostream>
#include <iomanip>
#include <numeric>
#include "random_utils.h"
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Randomized Response (Warner, 1965)
// Privacy-preserving data collection. Each respondent answers truthfully
// with probability p, and randomly otherwise. Provides plausible deniability:
// the server cannot know any individual's true answer.
//
// From aggregated randomized responses, the true population proportion
// can be estimated as: true_p = (observed_p - (1-p)/2) / (p - 1/2)
// ---------------------------------------------------------------------------

struct RandomizedResponseResult {
    std::vector<bool> raw_responses;  // what was actually reported
    int truthful_count;
    int total;
};

// Collect n randomized responses for a boolean attribute
// true_answers: the actual answers
// p: probability of answering truthfully (must be > 0.5 for utility)
inline RandomizedResponseResult collect_randomized_responses(
    const std::vector<bool>& true_answers,
    double p, std::mt19937& rng) {

    int n = static_cast<int>(true_answers.size());
    std::vector<bool> responses(n);
    std::uniform_real_distribution<double> unif(0.0, 1.0);
    int truthful = 0;

    for (int i = 0; i < n; ++i) {
        if (unif(rng) < p) {
            responses[i] = true_answers[i];
            truthful++;
        } else {
            responses[i] = !true_answers[i];  // flip
        }
    }
    return {responses, truthful, n};
}

// Estimate the true proportion from randomized responses
// If q = fraction of "yes" responses, then:
//   true_proportion = (q - (1-p)/2) / (p - 1/2)
inline double estimate_proportion(const std::vector<bool>& responses, double p) {
    int yes_count = 0;
    for (bool r : responses) yes_count += r;
    double q = static_cast<double>(yes_count) / responses.size();
    // Correct formula: true_p = (q - (1-p)) / (2p - 1)
    return (q - (1.0 - p)) / (2.0 * p - 1.0);
}

// ---------------------------------------------------------------------------
// Laplace Mechanism for Differential Privacy
// Adds Laplace noise calibrated to the query's sensitivity.
// For a query f: X -> R with global sensitivity Delta_f,
// the mechanism M(x) = f(x) + Lap(Delta_f / epsilon)
// satisfies epsilon-differential privacy.
// ---------------------------------------------------------------------------

// Sample from Laplace distribution
inline double laplace_sample(double b, std::mt19937& rng) {
    std::uniform_real_distribution<double> unif(-0.5, 0.5);
    double u = unif(rng);
    return -b * std::copysign(1.0, u) * std::log(1.0 - 2.0 * std::abs(u));
}

// Laplace mechanism: add noise to a numeric query
inline double laplace_mechanism(double true_value, double sensitivity,
                                double epsilon, std::mt19937& rng) {
    double scale = sensitivity / epsilon;
    return true_value + laplace_sample(scale, rng);
}

// Count query with Laplace mechanism
// Counts how many records satisfy a predicate, with differential privacy
inline double private_count(
    const std::vector<bool>& database,
    std::function<bool(int)> predicate,
    double epsilon, std::mt19937& rng) {

    double true_count = 0.0;
    for (size_t i = 0; i < database.size(); ++i) {
        if (predicate(static_cast<int>(i))) {
            true_count += database[i] ? 1.0 : 0.0;
        }
    }
    // Sensitivity of counting query = 1 (changing one record changes count by at most 1)
    return laplace_mechanism(true_count, 1.0, epsilon, rng);
}

// ---------------------------------------------------------------------------
// Randomized Aggregation: mean, sum, histogram with DP
// ---------------------------------------------------------------------------

// Private mean: add Laplace noise to sum, divide by n
inline double private_mean(
    const std::vector<double>& values,
    double value_bound,   // max |value_i|
    double epsilon,
    std::mt19937& rng) {

    double true_sum = 0.0;
    for (double v : values) true_sum += v;

    int n = static_cast<int>(values.size());
    // Sensitivity of sum = 2 * value_bound (one value can change from 0 to bound)
    double noisy_sum = laplace_mechanism(true_sum, 2.0 * value_bound, epsilon, rng);
    return noisy_sum / n;
}

// Private histogram: add noise to each bucket count
inline std::vector<double> private_histogram(
    const std::vector<int>& data,
    int num_buckets,
    double epsilon,
    std::mt19937& rng) {

    std::vector<double> counts(num_buckets, 0.0);
    for (int d : data) {
        if (d >= 0 && d < num_buckets) counts[d] += 1.0;
    }

    // Sensitivity of histogram = 1 (each record in exactly one bucket)
    double per_bucket_eps = epsilon / num_buckets;
    for (int i = 0; i < num_buckets; ++i) {
        counts[i] = laplace_mechanism(counts[i], 1.0, per_bucket_eps, rng);
    }
    return counts;
}

// ---------------------------------------------------------------------------
// Demonstration
// ---------------------------------------------------------------------------
inline void demonstrate_privacy() {
    std::mt19937 rng(42);

    println("=== Privacy-Preserving Data Collection ===\n");

    // --- Randomized Response ---
    println("--- 1. Randomized Response (Plausible Deniability) ---");
    {
        int n = 10000;
        double true_proportion = 0.3;  // 30% have the attribute
        double p = 0.75;               // answer truthfully 75% of the time

        // Generate true answers
        std::vector<bool> true_answers(n);
        std::uniform_real_distribution<double> unif(0.0, 1.0);
        int true_yes = 0;
        for (int i = 0; i < n; ++i) {
            true_answers[i] = (unif(rng) < true_proportion);
            if (true_answers[i]) true_yes++;
        }

        println("  True proportion: {:.4f} ({} out of {})",
                      (double)true_yes / n, true_yes, n);
        println("  Truthfulness probability p = {}", p);

        // Collect randomized responses
        auto result = collect_randomized_responses(true_answers, p, rng);

        int reported_yes = 0;
        for (bool r : result.raw_responses) reported_yes += r;
        println("  Reported 'yes' fraction: {:.4f} ({}/{})",
                      (double)reported_yes / n, reported_yes, n);

        // Estimate true proportion
        double estimated = estimate_proportion(result.raw_responses, p);
        println("  Estimated true proportion: {:.4f}", estimated);
        println("  Error: {:.4f}", std::abs(estimated - true_proportion));
        println("  Truthful answers given: {}/{} ({:.1f}%)",
                      result.truthful_count, n,
                      100.0 * result.truthful_count / n);
        println("  Server CANNOT determine individual answers!");
    }

    // --- Laplace Mechanism ---
    println("\n--- 2. Laplace Mechanism for Differential Privacy ---");
    {
        int n = 10000;
        std::vector<double> incomes(n);
        std::normal_distribution<double> dist(50000.0, 15000.0);
        for (int i = 0; i < n; ++i) {
            incomes[i] = std::max(10000.0, dist(rng));
        }

        double true_mean = 0.0;
        for (double v : incomes) true_mean += v;
        true_mean /= n;

        println("  Dataset: {} incomes, true mean = ${:.0f}", n, true_mean);

        // Try different epsilon values
        for (double eps : {0.1, 0.5, 1.0, 5.0}) {
            // Run 1000 trials
            double total_error = 0.0;
            for (int trial = 0; trial < 1000; ++trial) {
                double private_est = private_mean(incomes, 100000.0, eps, rng);
                total_error += std::abs(private_est - true_mean);
            }
            double avg_error = total_error / 1000;
            println("  epsilon={:.1f}: avg absolute error = ${:.0f} ({:.2f}%)",
                          eps, avg_error, 100.0 * avg_error / true_mean);
        }

        println("\n  Privacy-utility tradeoff:");
        println("    Smaller epsilon = more privacy, more noise");
        println("    Larger epsilon = less privacy, less noise");
    }

    // --- Private Histogram ---
    println("\n--- 3. Private Histogram (Age Distribution) ---");
    {
        int n = 5000;
        int buckets = 10;  // age decades
        std::vector<int> ages(n);
        std::normal_distribution<double> dist(40.0, 15.0);
        for (int i = 0; i < n; ++i) {
            ages[i] = std::clamp(static_cast<int>(dist(rng)), 0, 99) / 10;
        }

        // True histogram
        std::vector<double> true_hist(buckets, 0.0);
        for (int a : ages) true_hist[a]++;

        println("  Dataset: {} people, {} age buckets", n, buckets);

        double eps = 1.0;
        auto private_hist = private_histogram(ages, buckets, eps, rng);

        println("  Age Group  True Count  Private Count");
        for (int i = 0; i < buckets; ++i) {
            println("    {}0-{}9     {:>5.0f}         {:>8.1f}",
                          i, i, true_hist[i], private_hist[i]);
        }
    }
}

} // namespace ral
