// example_generic_reservoir.cpp
// Generic Template Reservoir Sampling with strings
//
// The templated version of reservoir_sample works with any copyable type,
// not just ints. This example demonstrates sampling from a stream of strings.
//
// Compile: g++ -std=c++23 -I../include -o example_generic_reservoir example_generic_reservoir.cpp

#include <ral/reservoir_sampling.h>
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <span>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Generic Reservoir Sampling (Strings) ===\n\n";

    // A "stream" of city names
    std::vector<std::string> cities = {
        "New York", "London", "Tokyo", "Paris", "Berlin",
        "Sydney", "Toronto", "Mumbai", "Sao Paulo", "Cairo",
        "Seoul", "Moscow", "Istanbul", "Bangkok", "Lima",
        "Nairobi", "Vienna", "Amsterdam", "Rome", "Lisbon"
    };

    int k = 5;
    int trials = 10000;

    std::cout << "Stream of " << cities.size() << " cities, sampling " << k << "\n\n";

    // Count how often each city is selected
    std::vector<int> freq(cities.size(), 0);
    for (int t = 0; t < trials; ++t) {
        std::span<const std::string> stream_span(cities);
        auto sample = reservoir_sample<std::string>(stream_span, k, rng);
        for (const auto& city : sample) {
            for (size_t i = 0; i < cities.size(); ++i) {
                if (cities[i] == city) {
                    freq[i]++;
                    break;
                }
            }
        }
    }

    double expected_prob = (double)k / cities.size();
    std::cout << "Expected inclusion probability: " << expected_prob << "\n\n";
    std::cout << "  City           Observed Prob\n";
    for (size_t i = 0; i < cities.size(); ++i) {
        double observed = (double)freq[i] / trials;
        std::cout << "  " << cities[i];
        // Pad to align
        for (size_t p = cities[i].size(); p < 16; ++p) std::cout << " ";
        std::cout << observed << "\n";
    }

    // Show one concrete sample
    std::span<const std::string> stream_span(cities);
    auto sample = reservoir_sample<std::string>(stream_span, k, rng);
    std::cout << "\nOne sample: ";
    for (const auto& city : sample) std::cout << city << "  ";
    std::cout << "\n";

    return 0;
}
