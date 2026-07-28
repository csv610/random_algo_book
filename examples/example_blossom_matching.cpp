/*
 * Maximum Matching in General Graphs (Augmenting Paths)
 *
 * Finds a maximum matching by repeatedly finding augmenting paths.
 * An augmenting path is a path that starts and ends at unmatched vertices,
 * alternating between unmatched and matched edges. Flipping all edges
 * along the path increases the matching size by 1.
 *
 * For bipartite graphs, BFS-based augmenting paths give a correct
 * maximum matching. For general graphs, the full Blossom algorithm
 * (Edmonds) is needed to handle odd cycles, but this BFS-based approach
 * works well for many practical cases.
 *
 * Time: O(V * E) per augmentation, O(V^2 * E) total.
 */

#include "ral/matchings.h"
#include <iostream>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Maximum Matching (Augmenting Paths)\n";
    std::cout << "========================================================\n\n";

    // Test 1: Complete bipartite K_{3,3} - perfect matching of size 3
    {
        std::cout << "K_{3,3} (bipartite, perfect matching = 3):\n";
        Graph g(6);
        for (int i = 0; i < 3; i++)
            for (int j = 3; j < 6; j++)
                g.add_edge(i, j);

        auto m = blossom_matching(g);
        std::cout << "  Size: " << m.size() << " (expected 3)\n\n";
    }

    // Test 2: Petersen graph - matching number 5
    {
        std::cout << "Petersen graph (matching number = 5):\n";
        Graph g(10);
        for (int i = 0; i < 5; i++) {
            g.add_edge(i, (i + 1) % 5);
            g.add_edge(i, i + 5);
            g.add_edge(i + 5, ((i + 2) % 5) + 5);
        }

        auto m = blossom_matching(g);
        std::cout << "  Size: " << m.size() << " (expected 5)\n\n";
    }

    // Test 3: Path graph P6 - maximum matching = 3
    {
        std::cout << "Path 0-1-2-3-4-5 (matching number = 3):\n";
        Graph g(6);
        for (int i = 0; i < 5; i++)
            g.add_edge(i, i + 1);

        auto m = blossom_matching(g);
        std::cout << "  Size: " << m.size() << " (expected 3)\n\n";
    }

    // Test 4: Star graph - matching number = 1
    {
        std::cout << "Star (center=0, leaves=1..5, matching number = 1):\n";
        Graph g(6);
        for (int i = 1; i < 6; i++)
            g.add_edge(0, i);

        auto m = blossom_matching(g);
        std::cout << "  Size: " << m.size() << " (expected 1)\n";
    }

    return 0;
}
