// example_weighted_reservoir.cpp
// Weighted Reservoir Sampling (Algorithm A-Res)
//
// Each element i has weight w_i. The probability of inclusion in the
// reservoir of size k is proportional to w_i. Uses the key function
// key_i = u_i^(1/w_i) where u_i ~ Uniform(0,1), and keeps the k
// elements with largest keys.
//
// Compile: g++ -std=c++23 -I../include -o example_weighted_reservoir example_weighted_reservoir.cpp

#include <ral/reservoir_sampling.h>
#include <iostream>
#include <vector>
#include <random>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Weighted Reservoir Sampling (A-Res) ===\n\n";

    // 5 items with different weights
    std::vector<int> stream = {0, 1, 2, 3, 4};
    std::vector<std::string> names = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};
    std::vector<double> weights = {1.0, 1.0, 1.0, 5.0, 5.0};

    int k = 2;
    int trials = 10000;

    double total_weight = 0.0;
    for (double w : weights) total_weight += w;

    std::cout << "Items and weights:\n";
    for (size_t i = 0; i < names.size(); ++i) {
        std::cout << "  " << names[i] << " (weight=" << weights[i]
                  << ", expected prob=" << k * weights[i] / total_weight << ")\n";
    }
    std::cout << "\nSampling " << k << " items from " << stream.size()
              << ", repeated " << trials << " times\n\n";

    // Count inclusion frequency
    std::vector<int> freq(5, 0);
    for (int t = 0; t < trials; ++t) {
        auto sample = weighted_reservoir_sample(stream, weights, k, rng);
        for (int x : sample) freq[x]++;
    }

    std::cout << "  Item       Weight  Expected Prob  Observed Prob\n";
    for (int i = 0; i < 5; ++i) {
        double expected_p = k * weights[i] / total_weight;
        double observed_p = (double)freq[i] / trials;
        std::cout << "  " << names[i] << "    " << weights[i]
                  << "         " << expected_p
                  << "          " << observed_p << "\n";
    }

    std::cout << "\nHigh-weight items (Date, Elderberry) are sampled much more often.\n";

    return 0;
}
