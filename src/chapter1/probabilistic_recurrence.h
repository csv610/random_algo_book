#pragma once

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include "random_utils.h"

namespace chapter1 {

// Randomized Find algorithm (select kth smallest)
// This is a Las Vegas algorithm - always correct, expected O(n) time
int randomized_find(std::vector<int>& arr, int k) {
    if (arr.size() == 1) return arr[0];
    
    // Pick random pivot
    int pivot_idx = rng().rand_int(0, static_cast<int>(arr.size()) - 1);
    int pivot = arr[pivot_idx];
    
    // Partition around pivot
    std::vector<int> less, equal, greater;
    for (int x : arr) {
        if (x < pivot) less.push_back(x);
        else if (x == pivot) equal.push_back(x);
        else greater.push_back(x);
    }
    
    if (k <= static_cast<int>(less.size())) {
        return randomized_find(less, k);
    } else if (k <= static_cast<int>(less.size() + equal.size())) {
        return pivot;
    } else {
        return randomized_find(greater, k - static_cast<int>(less.size()) - static_cast<int>(equal.size()));
    }
}

// Analyze the expected number of recursive calls
struct FindAnalysis {
    int total_calls;
    int max_depth;
    
    FindAnalysis() : total_calls(0), max_depth(0) {}
};

int randomized_find_analyzed(std::vector<int>& arr, int k, int depth, FindAnalysis& analysis) {
    analysis.total_calls++;
    analysis.max_depth = std::max(analysis.max_depth, depth);
    
    if (arr.size() == 1) return arr[0];
    
    int pivot_idx = rng().rand_int(0, static_cast<int>(arr.size()) - 1);
    int pivot = arr[pivot_idx];
    
    std::vector<int> less, equal, greater;
    for (int x : arr) {
        if (x < pivot) less.push_back(x);
        else if (x == pivot) equal.push_back(x);
        else greater.push_back(x);
    }
    
    if (k <= static_cast<int>(less.size())) {
        return randomized_find_analyzed(less, k, depth + 1, analysis);
    } else if (k <= static_cast<int>(less.size() + equal.size())) {
        return pivot;
    } else {
        return randomized_find_analyzed(greater, k - static_cast<int>(less.size()) - static_cast<int>(equal.size()), depth + 1, analysis);
    }
}

// Theorem 1.3: Probabilistic Recurrence
// If E[X] >= g(m) at each step, then expected steps to reach 1 is <= integral from 1 to n of dx/g(x)
void demonstrate_probabilistic_recurrence() {
    std::cout << "=== Probabilistic Recurrence (Section 1.4) ===\n\n";
    
    // Test the Find algorithm
    int n = 1000;
    int num_trials = 100;
    
    long long total_calls = 0;
    long long total_depth = 0;
    
    for (int trial = 0; trial < num_trials; trial++) {
        std::vector<int> arr(n);
        for (int i = 0; i < n; i++) {
            arr[i] = rng().rand_int(1, n * 10);
        }
        
        int k = rng().rand_int(1, n);
        
        FindAnalysis analysis;
        randomized_find_analyzed(arr, k, 0, analysis);
        
        total_calls += analysis.total_calls;
        total_depth += analysis.max_depth;
    }
    
    double avg_calls = static_cast<double>(total_calls) / num_trials;
    double avg_depth = static_cast<double>(total_depth) / num_trials;
    
    std::cout << "Randomized Find Algorithm Analysis:\n";
    std::cout << "Input size n = " << n << "\n";
    std::cout << "Average recursive calls: " << avg_calls << "\n";
    std::cout << "Average max depth: " << avg_depth << "\n";
    std::cout << "Theoretical bound: O(log n) = O(" << std::log2(n) << ")\n\n";
    
    // Demonstrate the geometric distribution connection
    std::cout << "Connection to Geometric Distribution:\n";
    std::cout << "At each step, the problem size reduces by a random amount X\n";
    std::cout << "where E[X] >= n/4 (for Find algorithm)\n";
    std::cout << "By Theorem 1.3, expected steps <= integral from 1 to n of dx/(x/4) = 4 ln n\n";
    std::cout << "For n = " << n << ": 4 ln n = " << 4.0 * std::log(n) << "\n\n";
}

// Geometric distribution demonstration
void demonstrate_geometric_distribution() {
    std::cout << "=== Geometric Distribution ===\n\n";
    
    // Simulate biased coin: probability p of heads
    double p = 0.3;
    int num_trials = 100000;
    
    long long total_flips = 0;
    
    for (int trial = 0; trial < num_trials; trial++) {
        int flips = 0;
        while (true) {
            flips++;
            if (rng().coin_flip(p)) {
                break;
            }
        }
        total_flips += flips;
    }
    
    double avg_flips = static_cast<double>(total_flips) / num_trials;
    
    std::cout << "Biased coin: P(heads) = " << p << "\n";
    std::cout << "Expected flips until first heads: 1/p = " << 1.0 / p << "\n";
    std::cout << "Empirical average over " << num_trials << " trials: " << avg_flips << "\n\n";
    
    std::cout << "Key Property: If success probability is p,\n";
    std::cout << "expected number of trials until first success is 1/p\n";
    std::cout << "This is used in converting Monte Carlo to Las Vegas algorithms\n\n";
}

// Analyze expected number of recursive calls as problem size reduces
void analyze_recursion_depth() {
    std::cout << "=== Recursion Depth Analysis ===\n\n";
    
    // For Find algorithm: at each step, expected reduction is at least n/4
    // So after k steps, expected problem size is at most n * (3/4)^k
    
    std::vector<int> sizes = {100, 1000, 10000, 100000};
    
    for (int n : sizes) {
        // Calculate expected depth to reduce to size 1
        // n * (3/4)^k <= 1
        // k >= log(n) / log(4/3)
        double expected_depth = std::log(n) / std::log(4.0 / 3.0);
        
        std::cout << "n = " << n << ": expected depth <= " << expected_depth << "\n";
    }
    
    std::cout << "\nGeneral formula: O(log n) levels of recursion\n";
    std::cout << "Each level does O(n) work total (across all branches)\n";
    std::cout << "Total expected time: O(n)\n\n";
}

} // namespace chapter1