// ============================================================
// Example: PRAM Simulation -- Parallel Algorithms (Chapter 12)
//
// A PRAM (Parallel Random Access Machine) is a theoretical model
// of parallel computation: many processors share a common memory
// and can read/write in lockstep. This example simulates PRAM
// algorithms on real hardware using std::jthread.
//
// Algorithms demonstrated:
//   1. Parallel Prefix Sum (Hillis-Steel):
//      Computes cumulative sums in O(log n) parallel steps using
//      p processors. Each processor doubles its stride each round.
//
//   2. Parallel List Ranking (Pointer Jumping):
//      Given a linked list via next-pointers, compute each node's
//      distance to the tail in O(log n) parallel rounds.
//
//   3. Parallel Connected Components (Label Propagation):
//      Each vertex starts with its own label; in each round every
//      vertex adopts the minimum label among its neighbours.
//      Converges in O(diameter) rounds.
//
// Compile: g++ -std=c++23 -I../include example_pram_simulation.cpp \
//          -o example_pram_simulation -pthread
// ============================================================

#include "ral/pram_simulation.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <chrono>

using namespace ral;

int main() {
    std::cout << "=== PRAM Simulation: Parallel Algorithms ===\n";

    // -------------------------------------------------------
    // 1. Parallel Prefix Sum (Hillis-Steel)
    // -------------------------------------------------------
    {
        std::cout << "\n--- Parallel Prefix Sum (Hillis-Steel) ---\n";
        std::cout << "  Problem: given [1,2,3,...,16], compute cumulative sums.\n";
        std::cout << "  Expected: [1,3,6,10,15,21,28,36,45,55,66,78,91,105,120,136]\n\n";

        constexpr int n = 16;
        std::vector<long long> input(n);
        std::iota(input.begin(), input.end(), 1);

        PRAM pram(16, n);

        auto t0 = std::chrono::steady_clock::now();
        auto parallel_result = pram.prefix_sum_parallel(input);
        auto t1 = std::chrono::steady_clock::now();

        auto t2 = std::chrono::steady_clock::now();
        auto seq_result = PRAM::prefix_sum_sequential(input);
        auto t3 = std::chrono::steady_clock::now();

        std::cout << "\n  Parallel result:  [";
        for (size_t i = 0; i < parallel_result.size(); ++i)
            std::cout << (i ? ", " : "") << parallel_result[i];
        std::cout << "]\n";

        std::cout << "  Sequential result:[";
        for (size_t i = 0; i < seq_result.size(); ++i)
            std::cout << (i ? ", " : "") << seq_result[i];
        std::cout << "]\n";

        bool match = (parallel_result == seq_result);
        std::cout << "  Results match: " << (match ? "YES" : "NO") << "\n";
        std::cout << "  Parallel time:   "
                  << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()
                  << " us\n";
        std::cout << "  Sequential time: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()
                  << " us\n";
    }

    // -------------------------------------------------------
    // 2. Parallel List Ranking (Pointer Jumping)
    // -------------------------------------------------------
    {
        std::cout << "\n--- Parallel List Ranking (Pointer Jumping) ---\n";
        std::cout << "  Problem: given a linked list, compute each node's distance to tail.\n";
        std::cout << "  List: 0 -> 1 -> 2 -> 3 -> 4 (tail)\n";
        std::cout << "  Expected ranks: [4, 3, 2, 1, 0]\n\n";

        // 0 -> 1 -> 2 -> 3 -> 4 (tail, next = -1)
        std::vector<int> next_ptr = {1, 2, 3, 4, -1};
        PRAM pram(5, 0);
        auto ranks = pram.list_ranking_parallel(next_ptr);

        std::cout << "\n  Verification: ";
        bool ok = true;
        for (int i = 0; i < 5; ++i) {
            int expected = 4 - i;
            if (ranks[i] != expected) { ok = false; break; }
        }
        std::cout << (ok ? "PASS" : "FAIL") << "\n";
    }

    // -------------------------------------------------------
    // 3. Parallel Connected Components
    // -------------------------------------------------------
    {
        std::cout << "\n--- Parallel Connected Components ---\n";
        std::cout << "  Problem: identify connected components in a graph.\n";
        std::cout << "  Graph: 0-1-2 (component A), 3-4 (component B)\n\n";

        // Adjacency list: 0-1-2 connected, 3-4 connected
        std::vector<std::vector<int>> adj = {
            {1},    // 0
            {0, 2}, // 1
            {1},    // 2
            {4},    // 3
            {3}     // 4
        };
        PRAM pram(5, 0);
        auto labels = pram.connected_components_parallel(adj);

        std::cout << "\n  Label assignments: ";
        for (int i = 0; i < 5; ++i)
            std::cout << "v" << i << "=" << labels[i] << " ";
        std::cout << "\n";

        // Verify: vertices in the same component share a label
        bool ok = (labels[0] == labels[1] && labels[1] == labels[2])
               && (labels[3] == labels[4])
               && (labels[0] != labels[3]);
        std::cout << "  Components correctly separated: " << (ok ? "YES" : "NO") << "\n";
    }

    // -------------------------------------------------------
    // 4. Larger prefix sum: practical scenario
    // -------------------------------------------------------
    {
        std::cout << "\n--- Practical: Parallel Prefix Sum for Scan/Exclusive sum ---\n";
        std::cout << "  Scenario: compute running totals of daily sales.\n";

        // Daily sales (units sold)
        std::vector<long long> sales = {12, 7, 5, 18, 3, 22, 9, 14};
        int n = static_cast<int>(sales.size());

        std::cout << "  Daily sales:  [";
        for (int i = 0; i < n; ++i) std::cout << (i ? ", " : "") << sales[i];
        std::cout << "]\n";

        PRAM pram(n, n);
        auto cumulative = pram.prefix_sum_parallel(sales);

        std::cout << "  Cumulative:   [";
        for (int i = 0; i < n; ++i) std::cout << (i ? ", " : "") << cumulative[i];
        std::cout << "]\n";

        std::cout << "  Total sales: " << cumulative[n - 1] << " units\n";

        // Verify manually
        std::vector<long long> expected = {12, 19, 24, 42, 45, 67, 76, 90};
        bool ok = (cumulative == expected);
        std::cout << "  Verification: " << (ok ? "PASS" : "FAIL") << "\n";
    }

    return 0;
}
