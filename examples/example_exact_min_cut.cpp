/*
 * Exact Minimum Cut via Max-Flow
 *
 * Computes the global minimum cut by running BFS-based max-flow (Edmonds-Karp)
 * from vertex 0 to every other vertex, and taking the minimum flow value.
 * This serves as ground truth to verify the randomized algorithms.
 *
 * Time complexity: O(n * (V*E)) = O(n^4) for dense graphs using BFS.
 */

#include "ral/min_cut.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Exact Min-Cut via Max-Flow (Verification)\n";
    std::cout << "========================================================\n\n";

    // Test 1: Simple cycle C4 - min-cut should be 2
    {
        Multigraph G(4);
        G.add_edge(0, 1); G.add_edge(1, 2);
        G.add_edge(2, 3); G.add_edge(3, 0);

        int exact = exact_min_cut(G, 4);
        std::cout << "C4 (cycle, 4 vertices): min-cut = " << exact << " (expected 2)\n";
    }

    // Test 2: Complete graph K5 - min-cut should be 4
    {
        Multigraph G(5);
        for (int i = 0; i < 5; i++)
            for (int j = i + 1; j < 5; j++)
                G.add_edge(i, j);

        int exact = exact_min_cut(G, 5);
        std::cout << "K5 (complete, 5 vertices): min-cut = " << exact << " (expected 4)\n";
    }

    // Test 3: Bridge graph - min-cut should be 1
    {
        Multigraph G(4);
        G.add_edge(0, 1); G.add_edge(0, 2);  // left side
        G.add_edge(2, 3); G.add_edge(3, 1);  // right side connected via bridge 2-3
        // Actually let's make a clear bridge:
        Multigraph H(4);
        H.add_edge(0, 1); H.add_edge(1, 2); H.add_edge(2, 3);
        H.add_edge(0, 2); // extra edge on left

        int exact = exact_min_cut(H, 4);
        std::cout << "Path with extra edge (4 vertices): min-cut = " << exact << " (expected 1)\n";
    }

    // Test 4: Comparison with Karger on larger random graph
    {
        Multigraph G = random_multigraph(20, 60);
        int exact = exact_min_cut(G, 20);
        int karger = karger_repeated(G, 200);

        std::cout << "\nRandom graph (n=20, m=60):\n";
        std::cout << "  Exact:   " << exact << "\n";
        std::cout << "  Karger:  " << karger << "\n";
        std::cout << "  Match:   " << (exact == karger ? "YES" : "NO") << "\n";
    }

    return 0;
}
