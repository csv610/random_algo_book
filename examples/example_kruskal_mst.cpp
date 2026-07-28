/*
 * Kruskal's Minimum Spanning Tree with Union-Find
 *
 * Sorts all edges by weight, then greedily adds each edge if it doesn't
 * create a cycle. Uses a Disjoint Set Union (DSU) data structure with
 * path compression and union by rank for near-O(1) amortized operations.
 *
 * Time complexity: O(E log E) for sorting + O(E * alpha(V)) for DSU.
 *
 * This serves as both a practical MST algorithm and as ground truth
 * for verifying the randomized KKT algorithm.
 */

#include "ral/mst.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Kruskal's MST with Union-Find\n";
    std::cout << "========================================================\n\n";

    // Build a weighted graph with known MST
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

    double mst_weight = kruskal_mst(G);
    std::cout << "Graph: 6 vertices, " << G.edges.size() << " edges\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "MST total weight: " << mst_weight << "\n\n";

    // Trace which edges are selected
    auto edges = G.edges;
    std::sort(edges.begin(), edges.end());

    DSU dsu(G.n);
    std::cout << "MST edges (sorted by weight):\n";
    double total = 0;
    int count = 0;
    for (const auto& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            std::cout << "  (" << e.u << "," << e.v << ") weight=" << e.weight << "\n";
            total += e.weight;
            count++;
        }
    }
    std::cout << "Total weight: " << total << " (edges: " << count << ")\n\n";

    // Verify with KKT randomized algorithm
    double kkt_weight = kkt_mst(G);
    std::cout << "KKT randomized MST weight: " << kkt_weight << "\n";
    std::cout << "Match: " << (std::abs(mst_weight - kkt_weight) < 1e-6 ? "YES" : "NO") << "\n";

    return 0;
}
