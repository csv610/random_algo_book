/*
 * Karger-Stein Recursive Min-Cut Algorithm
 *
 * An improvement over basic Karger: instead of contracting all the way down
 * to 2 vertices, contract only to ceil(n/sqrt(2)) vertices, then recurse
 * on two independent copies of the result. This reduces the failure
 * probability from O(1/n^2) to O(1/n^2 * sqrt(log n)), achieving the
 * near-optimal O(n^2 log n) running time.
 *
 * Reference: "An O(n^2 log n) Algorithm for the Minimum Cut Problem"
 *            - Karger and Stein, 1996
 */

#include "ral/min_cut.h"
#include <iostream>
#include <cmath>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Karger-Stein Recursive Min-Cut\n";
    std::cout << "========================================================\n\n";

    // Build the same test graph
    Multigraph G(6);
    G.add_edge(0, 1); G.add_edge(0, 2); G.add_edge(0, 3);
    G.add_edge(1, 2); G.add_edge(1, 4);
    G.add_edge(2, 3); G.add_edge(2, 5);
    G.add_edge(3, 4); G.add_edge(3, 5);
    G.add_edge(4, 5);

    int n = G.n;
    int exact = exact_min_cut(G, n);

    std::cout << "Graph: " << n << " vertices\n";
    std::cout << "Exact min-cut: " << exact << "\n\n";

    // Compare single Karger vs Karger-Stein over many runs
    constexpr int num_trials = 100;
    int success_karger = 0, success_ks = 0;

    for (int i = 0; i < num_trials; i++) {
        if (karger_min_cut_size(G) == exact) success_karger++;
        if (karger_stein_rec(G) == exact) success_ks++;
    }

    std::cout << "Over " << num_trials << " independent runs:\n";
    std::cout << "  Karger success:     " << success_karger << "/" << num_trials
              << " (" << 100.0 * success_karger / num_trials << "%)\n";
    std::cout << "  Karger-Stein success:" << success_ks << "/" << num_trials
              << " (" << 100.0 * success_ks / num_trials << "%)\n\n";

    // Karger-Stein with multiple outer repetitions
    int ks_result = karger_stein(G, 10);
    std::cout << "Karger-Stein (10 repetitions): " << ks_result << "\n";
    std::cout << "Matches exact: " << (ks_result == exact ? "YES" : "NO") << "\n";

    return 0;
}
