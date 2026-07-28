/*
 * BFS-based Max-Flow (Edmonds-Karp / Ford-Fulkerson with BFS)
 *
 * The Ford-Fulkerson method finds augmenting paths in the residual graph
 * and pushes flow along them. Using BFS to find the shortest augmenting
 * path gives the Edmonds-Karp algorithm with O(V * E^2) complexity.
 *
 * Used here as ground truth for min-cut verification:
 * by the max-flow min-cut theorem, the minimum s-t cut equals the
 * maximum flow. The global min-cut is the minimum over all s-t cuts.
 */

#include "ral/min_cut.h"
#include <iostream>
#include <vector>

using namespace ral;

void print_flow_network(const std::vector<std::vector<int>>& cap, int n) {
    std::cout << "  Edge capacities:\n";
    for (int u = 0; u < n; u++)
        for (int v = u + 1; v < n; v++)
            if (cap[u][v] > 0)
                std::cout << "    " << u << " -> " << v << " : capacity " << cap[u][v] << "\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  BFS-based Max-Flow (Edmonds-Karp)\n";
    std::cout << "========================================================\n\n";

    // Classic example: 6-vertex network
    //   s=0, t=5
    //   0->1 (16), 0->2 (13), 1->2 (4), 1->3 (12)
    //   2->1 (10), 2->4 (14), 3->2 (9), 3->5 (20)
    //   4->3 (7), 4->5 (4)
    constexpr int n = 6;
    std::vector<std::vector<int>> cap(n, std::vector<int>(n, 0));

    auto add = [&](int u, int v, int c) { cap[u][v] = c; };
    add(0, 1, 16); add(0, 2, 13);
    add(1, 2, 4);  add(1, 3, 12);
    add(2, 1, 10); add(2, 4, 14);
    add(3, 2, 9);  add(3, 5, 20);
    add(4, 3, 7);  add(4, 5, 4);

    std::cout << "Network (s=0, t=5):\n";
    print_flow_network(cap, n);

    int flow = max_flow_bfs(cap, 0, 5);
    std::cout << "\n  Max flow (0->5): " << flow << " (expected 23)\n";

    // Simple linear chain: 0->1->2->3->4, capacities all 5
    std::cout << "\n--- Linear chain (5 vertices, cap=5 each) ---\n";
    std::vector<std::vector<int>> cap2(5, std::vector<int>(5, 0));
    for (int i = 0; i < 4; i++) cap2[i][i + 1] = 5;
    int flow2 = max_flow_bfs(cap2, 0, 4);
    std::cout << "  Max flow (0->4): " << flow2 << " (expected 5)\n";

    // Parallel paths
    std::cout << "\n--- Two parallel paths (6 vertices) ---\n";
    std::vector<std::vector<int>> cap3(6, std::vector<int>(6, 0));
    cap3[0][1] = 10; cap3[1][5] = 10;
    cap3[0][2] = 7;  cap3[2][5] = 7;
    int flow3 = max_flow_bfs(cap3, 0, 5);
    std::cout << "  Max flow (0->5): " << flow3 << " (expected 17)\n";

    // Verify min-cut relationship
    std::cout << "\n--- Min-cut via max-flow ---\n";
    Multigraph G(5);
    G.add_edge(0, 1); G.add_edge(0, 2); G.add_edge(1, 2);
    G.add_edge(1, 3); G.add_edge(2, 4); G.add_edge(3, 4);
    int exact_cut = exact_min_cut(G, 5);
    std::cout << "  Graph min-cut: " << exact_cut << "\n";
    std::cout << "  (min-cut <= min s-t flow for any s,t)\n";

    return 0;
}
