/*
 * Karger-Klein-Tarjan (KKT) Randomized MST Algorithm
 *
 * A Las Vegas algorithm for MST that runs in O(m) expected time on average
 * graphs (though the worst case is higher). The key ideas:
 *
 *   1. Sample edges independently with probability 1/2
 *   2. Recursively compute MST of the sample
 *   3. Find all F-light edges (edges no heavier than the max edge on the
 *      forest path between its endpoints)
 *   4. Run Boruvka steps to contract the graph
 *   5. Recurse on the contracted graph
 *
 * Reference: "Randomized Minimum Spanning Tree Algorithms"
 *            - Karger, Klein, Tarjan, 1995
 */

#include "ral/mst.h"
#include <iostream>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  KKT Randomized MST Algorithm\n";
    std::cout << "========================================================\n\n";

    // Test 1: Small hand-crafted graph
    {
        WeightedGraph G(6);
        G.add_edge(0, 1, 4.0);
        G.add_edge(0, 2, 3.0);
        G.add_edge(1, 2, 1.0);
        G.add_edge(1, 3, 2.0);
        G.add_edge(2, 3, 8.0);
        G.add_edge(2, 4, 5.0);
        G.add_edge(3, 4, 7.0);
        G.add_edge(3, 5, 6.0);
        G.add_edge(4, 5, 9.0);

        double kruskal_w = kruskal_mst(G);
        double kkt_w = kkt_mst(G);

        std::cout << std::fixed << std::setprecision(1);
        std::cout << "Test 1: Small graph\n";
        std::cout << "  Kruskal: " << kruskal_w << "\n";
        std::cout << "  KKT:     " << kkt_w << "\n";
        std::cout << "  Match:   " << (std::abs(kruskal_w - kkt_w) < 1e-6 ? "YES" : "NO") << "\n\n";
    }

    // Test 2: Timing comparison on random graphs
    std::cout << "Test 2: Timing comparison\n";
    for (int n : {50, 100, 200, 500}) {
        double p = std::min(1.0, 6.0 / n);
        auto G = random_weighted_graph(n, p);

        auto t1 = std::chrono::high_resolution_clock::now();
        [[maybe_unused]] double kruskal_w = kruskal_mst(G);
        auto t2 = std::chrono::high_resolution_clock::now();
        double kkt_w = kkt_mst(G);
        auto t3 = std::chrono::high_resolution_clock::now();

        double kruskal_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        double kkt_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

        bool match = verify_mst(G, kkt_w);
        std::cout << "  n=" << n << " m=" << G.edges.size()
                  << ": Kruskal=" << std::fixed << std::setprecision(1) << kruskal_ms
                  << "ms, KKT=" << kkt_ms << "ms, match=" << (match ? "YES" : "NO") << "\n";
    }

    return 0;
}
