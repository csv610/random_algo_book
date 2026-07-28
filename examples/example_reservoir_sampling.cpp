// example_reservoir_sampling.cpp
// Reservoir Sampling (Vitter's Algorithm R)
//
// Maintains a uniformly random sample of size k from a data stream of
// unknown or infinite length. Each element has probability k/n of being
// selected at any point. Time: O(n), Space: O(k).
//
// Compile: g++ -std=c++23 -I../include -o example_reservoir_sampling example_reservoir_sampling.cpp

#include <ral/reservoir_sampling.h>
#include <iostream>
#include <vector>
#include <random>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    // Simulate a stream of 1000 elements
    int n = 1000;
    std::vector<int> stream(n);
    for (int i = 0; i < n; ++i) stream[i] = i;

    int k = 10;
    int trials = 10000;

    std::cout << "=== Basic Reservoir Sampling ===\n\n";
    std::cout << "Stream size: " << n << ", Sample size: " << k << "\n";
    std::cout << "Expected inclusion probability per element: " << (double)k / n << "\n\n";

    // Count how often each element appears across many trials
    std::vector<int> freq(n, 0);
    for (int t = 0; t < trials; ++t) {
        auto sample = reservoir_sample(stream, k, rng);
        for (int x : sample) freq[x]++;
    }

    // Verify uniformity: each element should appear ~k/n * trials times
    double expected = (double)k / trials * n;  // no: expected frequency = trials * k/n
    double expected_prob = (double)k / n;

    std::cout << "Observed inclusion probabilities (first 20 elements):\n";
    std::cout << "  Element  Expected   Observed\n";
    for (int i = 0; i < 20; ++i) {
        double observed = (double)freq[i] / trials;
        std::cout << "    " << i << "      " << expected_prob
                  << "       " << observed << "\n";
    }

    // Compute max deviation from expected
    double max_dev = 0.0;
    for (int i = 0; i < n; ++i) {
        double observed = (double)freq[i] / trials;
        max_dev = std::max(max_dev, std::abs(observed - expected_prob));
    }
    std::cout << "\nMax deviation from expected probability: " << max_dev << "\n";
    std::cout << "(Should be small, confirming uniform sampling)\n";

    // Show a single sample
    std::cout << "\nA single sample of size " << k << ": ";
    auto sample = reservoir_sample(stream, k, rng);
    for (int x : sample) std::cout << x << " ";
    std::cout << "\n";

    return 0;
}
