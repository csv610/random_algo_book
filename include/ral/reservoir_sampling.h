#pragma once

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <functional>
#include "random_utils.h"

namespace ral {

// ---------------------------------------------------------------------------
// Reservoir Sampling (Vitter's Algorithm R, 1985)
// Maintains a uniformly random sample of size k from a data stream of
// unknown or infinite length. Each element has probability k/n of being
// in the sample at any point.
//
// Time: O(n) for n elements, O(1) per element after initialization.
// Space: O(k)
// ---------------------------------------------------------------------------

inline std::vector<int> reservoir_sample(
    const std::vector<int>& stream, int k, std::mt19937& rng) {

    int n = static_cast<int>(stream.size());
    if (k <= 0) return {};
    if (k >= n) return stream;

    std::vector<int> reservoir(stream.begin(), stream.begin() + k);

    for (int i = k; i < n; ++i) {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(rng);
        if (j < k) {
            reservoir[j] = stream[i];
        }
    }
    return reservoir;
}

// Generic version: works with any copyable type
template<typename T>
inline std::vector<T> reservoir_sample(
    std::span<const T> stream, int k, std::mt19937& rng) {

    int n = static_cast<int>(stream.size());
    if (k <= 0) return {};
    if (k >= n) return std::vector<T>(stream.begin(), stream.end());

    std::vector<T> reservoir(stream.begin(), stream.begin() + k);

    for (int i = k; i < n; ++i) {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(rng);
        if (j < k) {
            reservoir[j] = stream[i];
        }
    }
    return reservoir;
}

// Reservoir sampling with weighted selection (Algorithm A-Res)
// Each element i has weight w_i. Probability of inclusion proportional to w_i.
struct WeightedElement {
    int index;
    double weight;
};

inline std::vector<int> weighted_reservoir_sample(
    const std::vector<int>& stream,
    const std::vector<double>& weights,
    int k, std::mt19937& rng) {

    int n = static_cast<int>(stream.size());
    if (k <= 0 || n == 0) return {};

    // Min-heap of (priority, index) pairs using std::greater
    using Pair = std::pair<double, int>;
    std::vector<Pair> reservoir;
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    auto cmp = std::greater<Pair>{};

    for (int i = 0; i < n; ++i) {
        double u = uniform(rng);
        double key = std::pow(u, 1.0 / weights[i]);

        if (static_cast<int>(reservoir.size()) < k) {
            reservoir.push_back({key, i});
            std::push_heap(reservoir.begin(), reservoir.end(), cmp);
        } else if (key > reservoir.front().first) {
            std::pop_heap(reservoir.begin(), reservoir.end(), cmp);
            reservoir.back() = {key, i};
            std::push_heap(reservoir.begin(), reservoir.end(), cmp);
        }
    }

    std::vector<int> result;
    for (auto& [key, idx] : reservoir) {
        result.push_back(stream[idx]);
    }
    return result;
}

// Demonstration
inline void demonstrate_reservoir_sampling() {
    std::mt19937 rng(42);

    println("=== Reservoir Sampling ===\n");

    // Generate a stream of 1000 elements (0..999)
    int n = 1000;
    std::vector<int> stream(n);
    for (int i = 0; i < n; ++i) stream[i] = i;

    // Draw reservoirs of size k=10
    int k = 10;
    int trials = 10000;

    // Count frequency of each element
    std::vector<int> freq(n, 0);
    for (int t = 0; t < trials; ++t) {
        auto sample = reservoir_sample(stream, k, rng);
        for (int x : sample) freq[x]++;
    }

    println("--- Uniform Reservoir Sampling (k={}, n={}, trials={}) ---", k, n, trials);
    println("  Each element should appear with probability k/n = {:.4f}", (double)k / n);
    println("  Observed frequencies for first 20 elements:");
    print("    ");
    for (int i = 0; i < 20; ++i) {
        double empirical = (double)freq[i] / trials;
        print("{:4.3f} ", empirical);
    }
    println("");

    double expected = (double)k / n;
    double max_dev = 0.0;
    for (int i = 0; i < n; ++i) {
        double empirical = (double)freq[i] / trials;
        max_dev = std::max(max_dev, std::abs(empirical - expected));
    }
    println("  Max deviation from expected: {:.4f}", max_dev);

    // Weighted reservoir sampling demo
    println("\n--- Weighted Reservoir Sampling ---");
    std::vector<int> wstream = {0, 1, 2, 3, 4};
    std::vector<double> weights = {1.0, 1.0, 1.0, 5.0, 5.0};
    int wk = 2;
    int wtrials = 10000;
    std::vector<int> wfreq(5, 0);

    for (int t = 0; t < wtrials; ++t) {
        auto sample = weighted_reservoir_sample(wstream, weights, wk, rng);
        for (int x : sample) wfreq[x]++;
    }

    double total_weight = 0.0;
    for (double w : weights) total_weight += w;

    println("  Weights: [1, 1, 1, 5, 5], k={}, trials={}", wk, wtrials);
    println("  Element  Inclusion Prob (expected)  Inclusion Prob (observed)");
    for (int i = 0; i < 5; ++i) {
        double expected_p = wk * weights[i] / total_weight;
        double observed_p = (double)wfreq[i] / wtrials;
        println("    {}         {:.4f}                   {:.4f}",
                      i, expected_p, observed_p);
    }
}

} // namespace ral
