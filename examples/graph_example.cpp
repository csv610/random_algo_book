#include "ral.h"
#include <iostream>
#include <vector>
#include <iomanip>

int main() {
    std::cout << "========================================================\n";
    std::cout << "           RAL Graph Algorithms Examples\n";
    std::cout << "========================================================\n\n";

    // ----------------------------------------------------------------
    // 1. Karger-Stein Min-Cut on a Multigraph
    // ----------------------------------------------------------------
    std::cout << "--- 1. Karger-Stein Min-Cut ---\n";
    // We create a graph with two cliques connected by a bridge
    // Clique A: {0, 1, 2, 3}, Clique B: {4, 5, 6, 7}
    // Bridge edge: (3, 4)
    ral::Multigraph mg(8);
    // Clique A
    mg.add_edge(0, 1); mg.add_edge(0, 2); mg.add_edge(0, 3);
    mg.add_edge(1, 2); mg.add_edge(1, 3); mg.add_edge(2, 3);
    // Clique B
    mg.add_edge(4, 5); mg.add_edge(4, 6); mg.add_edge(4, 7);
    mg.add_edge(5, 6); mg.add_edge(5, 7); mg.add_edge(6, 7);
    // Bridge (Min-cut should be 1, separating Clique A and Clique B)
    mg.add_edge(3, 4);

    int min_cut = ral::karger_stein(mg, 10);
    std::cout << "  Karger-Stein found Min-Cut size: " << min_cut << " (Expected: 1)\n\n";

    // ----------------------------------------------------------------
    // 2. Karger-Klein-Tarjan (KKT) Minimum Spanning Tree
    // ----------------------------------------------------------------
    std::cout << "--- 2. Karger-Klein-Tarjan (KKT) MST ---\n";
    ral::WeightedGraph wg(5);
    wg.add_edge(0, 1, 2.0);
    wg.add_edge(0, 3, 6.0);
    wg.add_edge(1, 2, 3.0);
    wg.add_edge(1, 3, 8.5);
    wg.add_edge(1, 4, 5.0);
    wg.add_edge(2, 4, 7.0);
    wg.add_edge(3, 4, 9.0);

    double mst_weight = ral::kkt_mst(wg);
    std::cout << "  KKT MST expected weight: " << mst_weight << " (Expected: 16.0)\n\n";

    // ----------------------------------------------------------------
    // 3. Seidel's All-Pairs Shortest Paths (APSP)
    // ----------------------------------------------------------------
    std::cout << "--- 3. Seidel's APSP (Unweighted, Undirected) ---\n";
    ral::BoolMatrix adj(4, std::vector<bool>(4, false));
    // 0 - 1 - 2
    // |       |
    // 3 -------
    adj[0][1] = adj[1][0] = true;
    adj[1][2] = adj[2][1] = true;
    adj[2][3] = adj[3][2] = true;
    adj[0][3] = adj[3][0] = true;

    auto dist_matrix = ral::seidel_apsp_unweighted(adj);
    std::cout << "  APSP Distance Matrix:\n";
    for (int i = 0; i < 4; ++i) {
        std::cout << "    ";
        for (int j = 0; j < 4; ++j) {
            std::cout << dist_matrix[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    // ----------------------------------------------------------------
    // 4. Luby's Parallel Maximal Independent Set (MIS)
    // ----------------------------------------------------------------
    std::cout << "--- 4. Luby's Parallel MIS ---\n";
    ral::Graph g(6);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    g.add_edge(4, 5);
    g.add_edge(5, 0);

    auto mis = ral::mis_luby_parallel(g);
    std::cout << "  MIS Vertices: ";
    for (int v : mis.vertices()) {
        std::cout << v << " ";
    }
    std::cout << "\n\n";

    // ----------------------------------------------------------------
    // 5. Randomized Matchings
    // ----------------------------------------------------------------
    std::cout << "--- 5. Randomized Matchings ---\n";
    ral::Graph g_match(6);
    g_match.add_edge(0, 1);
    g_match.add_edge(1, 2);
    g_match.add_edge(2, 3);
    g_match.add_edge(3, 4);
    g_match.add_edge(4, 5);

    auto match_greedy = ral::random_maximal_matching(g_match);
    auto match_blossom = ral::blossom_matching(g_match);
    auto match_approx = ral::approximate_max_matching(g_match);

    std::cout << "  Random Maximal Matching size: " << match_greedy.edges.size() << "\n";
    std::cout << "  Exact Blossom Max Matching size: " << match_blossom.edges.size() << "\n";
    std::cout << "  Approximate Max Matching size: " << match_approx.edges.size() << "\n\n";

    std::cout << "========================================================\n\n";
    return 0;
}
