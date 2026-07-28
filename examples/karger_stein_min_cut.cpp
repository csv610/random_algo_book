#include "ral.h"
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Example: Karger-Stein Recursive Min-Cut\n";
    std::cout << "========================================================\n\n";

    // Simple graph with known min-cut
    ral::Multigraph g(6);
    g.add_edge(0, 1); g.add_edge(0, 2); g.add_edge(0, 3);
    g.add_edge(1, 2); g.add_edge(1, 4);
    g.add_edge(2, 3); g.add_edge(2, 4);
    g.add_edge(3, 5); g.add_edge(4, 5);

    std::cout << "Graph: 6 vertices, " << g.total_edges() << " edges\n";
    int exact = ral::exact_min_cut(g, g.n);
    std::cout << "Exact min-cut: " << exact << "\n";
    std::cout << "Karger-Stein (5 reps): " << ral::karger_stein(g, 5) << "\n\n";

    // Timing on small random graphs
    std::cout << "Timing on small random graphs:\n";
    for (int n : {10, 20, 30}) {
        int m = 2 * n;
        auto G = ral::random_multigraph(n, m);
        int exact_val = ral::exact_min_cut(G, n);

        auto t1 = std::chrono::high_resolution_clock::now();
        int karger_val = ral::karger_repeated(G, n * n / 2);
        auto t2 = std::chrono::high_resolution_clock::now();
        int ks_val = ral::karger_stein(G, 3);
        auto t3 = std::chrono::high_resolution_clock::now();

        double karger_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        double ks_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

        std::cout << "  n=" << n << ": Exact=" << exact_val
                  << ", Karger=" << karger_val << " (" << karger_ms << "ms)"
                  << ", Karger-Stein=" << ks_val << " (" << ks_ms << "ms)\n";
    }

    return 0;
}