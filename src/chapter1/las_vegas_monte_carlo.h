#pragma once

#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <cmath>
#include "random_utils.h"

namespace chapter1 {

// ============================================================
// Las Vegas Algorithm: Randomized QuickSort
// Always gives correct result, running time is random
// ============================================================

int partition(std::vector<int>& arr, int low, int high) {
    int pivot_idx = rng().rand_int(low, high);
    std::swap(arr[pivot_idx], arr[high]);
    int pivot = arr[high];
    
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

void randomized_quicksort(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        randomized_quicksort(arr, low, pi - 1);
        randomized_quicksort(arr, pi + 1, high);
    }
}

// Las Vegas algorithm: always correct, running time is O(n log n) expected
void demonstrate_las_vegas() {
    std::cout << "=== Las Vegas Algorithm: Randomized QuickSort ===\n\n";
    
    // Test with different sizes
    std::vector<int> sizes = {10, 100, 1000, 10000};
    
    for (int n : sizes) {
        std::vector<int> arr(n);
        for (int i = 0; i < n; i++) {
            arr[i] = rng().rand_int(1, n * 10);
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        randomized_quicksort(arr, 0, n - 1);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Verify sorted
        bool sorted = std::is_sorted(arr.begin(), arr.end());
        
        std::cout << "n = " << n << ": " << duration.count() << " us, sorted: " 
                  << (sorted ? "YES" : "NO") << "\n";
    }
    
    std::cout << "\nKey Property: Always correct, running time varies per run\n";
    std::cout << "Expected time: O(n log n)\n\n";
}

// ============================================================
// Monte Carlo Algorithm: Randomized Min-Cut
// May give wrong answer, but probability of error is bounded
// ============================================================

// Randomized min-cut with one-sided error (like RP)
// Returns a cut that is at least as large as the true min-cut
int randomized_min_cut_monte_carlo(const std::vector<std::pair<int, int>>& edges, int n, int num_trials) {
    int min_cut = INT_MAX;
    
    for (int trial = 0; trial < num_trials; trial++) {
        // Simple randomized approach: randomly select edges
        std::vector<int> vertex_component(n);
        std::iota(vertex_component.begin(), vertex_component.end(), 0);
        
        // Union-Find data structure
        auto find = [&](int x) -> int {
            while (vertex_component[x] != x) {
                vertex_component[x] = vertex_component[vertex_component[x]];
                x = vertex_component[x];
            }
            return x;
        };
        
        auto unite = [&](int x, int y) {
            int root_x = find(x);
            int root_y = find(y);
            if (root_x != root_y) {
                vertex_component[root_x] = root_y;
            }
        };
        
        // Randomly permute edges and contract until 2 components
        std::vector<int> edge_indices(edges.size());
        std::iota(edge_indices.begin(), edge_indices.end(), 0);
        rng().shuffle(edge_indices);
        
        int components = n;
        for (int idx : edge_indices) {
            if (components <= 2) break;
            auto [u, v] = edges[idx];
            if (find(u) != find(v)) {
                unite(u, v);
                components--;
            }
        }
        
        // Count edges crossing the cut
        int cut_size = 0;
        for (const auto& [u, v] : edges) {
            if (find(u) != find(v)) {
                cut_size++;
            }
        }
        
        min_cut = std::min(min_cut, cut_size);
    }
    
    return min_cut;
}

// Monte Carlo with two-sided error
// Returns an estimate that may be too high or too low
double estimate_pi_monte_carlo(int num_samples) {
    int inside_circle = 0;
    
    for (int i = 0; i < num_samples; i++) {
        double x = rng().rand_double(-1.0, 1.0);
        double y = rng().rand_double(-1.0, 1.0);
        
        if (x * x + y * y <= 1.0) {
            inside_circle++;
        }
    }
    
    return 4.0 * inside_circle / num_samples;
}

void demonstrate_monte_carlo() {
    std::cout << "=== Monte Carlo Algorithms ===\n\n";
    
    // Example 1: Pi estimation (two-sided error)
    std::cout << "1. Estimating Pi using Monte Carlo (two-sided error):\n";
    std::vector<int> sample_sizes = {100, 1000, 10000, 100000, 1000000};
    
    for (int n : sample_sizes) {
        double pi_estimate = estimate_pi_monte_carlo(n);
        double error = std::abs(pi_estimate - M_PI);
        std::cout << "   Samples: " << n << ", Estimate: " << pi_estimate 
                  << ", Error: " << error << "\n";
    }
    
    // Example 2: Randomized min-cut (one-sided error)
    std::cout << "\n2. Randomized Min-Cut (one-sided error):\n";
    
    // Create a simple graph
    int n = 6;
    std::vector<std::pair<int, int>> edges = {
        {0, 1}, {0, 2}, {0, 3},
        {1, 2}, {1, 4},
        {2, 3}, {2, 4},
        {3, 5},
        {4, 5}
    };
    
    std::cout << "   Graph: " << n << " vertices, " << edges.size() << " edges\n";
    
    // Run with different numbers of trials
    std::vector<int> trial_counts = {1, 5, 10, 50};
    for (int trials : trial_counts) {
        int result = randomized_min_cut_monte_carlo(edges, n, trials);
        std::cout << "   Trials: " << trials << ", Min-cut found: " << result << "\n";
    }
    
    std::cout << "\nKey Properties:\n";
    std::cout << "- Running time is deterministic (polynomial)\n";
    std::cout << "- Solution quality is random\n";
    std::cout << "- Error probability can be reduced by repetition\n\n";
}

// ============================================================
// Converting Monte Carlo to Las Vegas
// Using the verification technique from Exercise 1.3
// ============================================================

// Simulate a Monte Carlo algorithm with success probability p
// Returns (result, is_correct)
std::pair<int, bool> monte_carlo_algorithm(int input) {
    // Simulated algorithm: correct with probability 2/3
    bool success = rng().coin_flip(2.0 / 3.0);
    if (success) {
        return {input * 2, true};  // Correct answer
    } else {
        return {input * 3, false}; // Wrong answer
    }
}

// Verify the result (simulated)
bool verify_result(int input, int result) {
    return result == input * 2;  // Correct answer is input * 2
}

// Convert Monte Carlo to Las Vegas using verification
std::pair<int, int> monte_carlo_to_las_vegas(int input) {
    int attempts = 0;
    
    while (true) {
        attempts++;
        auto [result, correct] = monte_carlo_algorithm(input);
        
        if (verify_result(input, result)) {
            return {result, attempts};
        }
    }
}

void demonstrate_conversion() {
    std::cout << "=== Converting Monte Carlo to Las Vegas ===\n\n";
    
    int num_trials = 100;
    long long total_attempts = 0;
    
    for (int i = 0; i < num_trials; i++) {
        int input = rng().rand_int(1, 100);
        auto [result, attempts] = monte_carlo_to_las_vegas(input);
        total_attempts += attempts;
    }
    
    double avg_attempts = static_cast<double>(total_attempts) / num_trials;
    
    std::cout << "Monte Carlo algorithm success probability: 2/3\n";
    std::cout << "Expected attempts to get correct result: " << 1.0 / (2.0/3.0) << "\n";
    std::cout << "Average attempts observed: " << avg_attempts << "\n\n";
    
    std::cout << "Key Insight: Geometric distribution\n";
    std::cout << "If success probability is p, expected attempts = 1/p\n";
    std::cout << "This converts Monte Carlo to Las Vegas (always correct)\n\n";
}

} // namespace chapter1