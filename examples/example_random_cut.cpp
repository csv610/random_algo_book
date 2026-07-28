/*
 * Random Cut Technique
 *
 * The random cut technique is the core subroutine behind Karger's algorithm.
 * Given a multigraph, repeatedly pick a uniformly random edge and contract it
 * (merge its two endpoints). When only 2 vertices remain, the remaining
 * edges form a cut of the original graph.
 *
 * This file demonstrates:
 *   - Single random contraction
 *   - Repeated random cuts and their success probability
 *   - How the technique scales with graph size
 */

#include "ral/min_cut.h"
#include <iostream>
#include <cmath>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Random Cut Technique\n";
    std::cout << "========================================================\n\n";

    // Build a graph with known min-cut = 3
    Multigraph G(5);
    // Triangle 0-1-2
    G.add_edge(0, 1); G.add_edge(0, 1); // double edge
    G.add_edge(1, 2); G.add_edge(1, 2);
    G.add_edge(0, 2);
    // Attach 3,4
    G.add_edge(2, 3); G.add_edge(2, 3); G.add_edge(2, 3);
    G.add_edge(3, 4);

    int n = G.n;
    int exact = exact_min_cut(G, n);

    std::cout << "Graph: " << n << " vertices, " << G.total_edges() << " edges\n";
    std::cout << "Exact min-cut: " << exact << "\n\n";

    // Show several individual cuts
    std::cout << "10 individual random cut results:\n  ";
    for (int i = 0; i < 10; i++)
        std::cout << karger_min_cut_size(G) << " ";
    std::cout << "\n\n";

    // Success probability analysis
    double p_success = 2.0 / (n * (n - 1));
    std::cout << "P(success one cut): " << p_success << "\n";

    int trials = n * n / 2;
    double p_fail_all = std::pow(1.0 - p_success, trials);
    std::cout << "After " << trials << " trials, P(all fail): " << p_fail_all << "\n\n";

    // Empirical success rate
    int exact_count = 0;
    constexpr int num_runs = 200;
    for (int i = 0; i < num_runs; i++) {
        if (karger_min_cut_size(G) == exact) exact_count++;
    }
    std::cout << "Empirical success rate: " << exact_count << "/" << num_runs
              << " = " << std::fixed << std::setprecision(3)
              << 100.0 * exact_count / num_runs << "%\n";
    std::cout << "Theoretical per-run:   " << std::setprecision(3)
              << 100.0 * p_success << "%\n\n";

    // Scaling with graph size
    std::cout << "Scaling analysis:\n";
    for (int sz : {10, 20, 30}) {
        auto H = random_multigraph(sz, 3 * sz);
        int exact_h = exact_min_cut(H, sz);
        int repeated = karger_repeated(H, sz * sz / 2);
        std::cout << "  n=" << sz << ": exact=" << exact_h
                  << ", karger=" << repeated
                  << ", match=" << (exact_h == repeated ? "YES" : "NO") << "\n";
    }

    return 0;
}
