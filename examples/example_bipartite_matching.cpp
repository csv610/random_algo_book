/*
 * Maximum Bipartite Matching
 *
 * In a bipartite graph, the maximum matching equals the minimum vertex
 * cover (König's theorem). This example demonstrates several matching
 * algorithms on bipartite graphs and compares their quality.
 *
 * Algorithms shown:
 *   - Random maximal matching (fast, 1/2-approximate)
 *   - Augmenting paths (exact maximum matching)
 *   - Random greedy (1/2-approximate)
 */

#include "ral/matchings.h"
#include "ral/random_utils.h"
#include <iostream>

using namespace ral;

Graph make_bipartite(int left, int right, double p) {
    Graph g(left + right);
    for (int i = 0; i < left; i++)
        for (int j = left; j < left + right; j++)
            if (rng().coin_flip(p))
                g.add_edge(i, j);
    return g;
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Bipartite Matching Comparison\n";
    std::cout << "========================================================\n\n";

    // K_{4,4}: perfect matching of size 4
    {
        std::cout << "K_{4,4} (complete bipartite, perfect matching = 4):\n";
        Graph g(8);
        for (int i = 0; i < 4; i++)
            for (int j = 4; j < 8; j++)
                g.add_edge(i, j);

        auto rm = random_maximal_matching(g);
        auto bm = blossom_matching(g);
        auto am = approximate_max_matching(g);

        std::cout << "  Random maximal: " << rm.size() << "\n";
        std::cout << "  Maximum (aug):  " << bm.size() << " (expected 4)\n";
        std::cout << "  1/2-approx:     " << am.size() << "\n\n";
    }

    // Random bipartite graph
    {
        std::cout << "Random bipartite (10+10 vertices, p=0.4):\n";
        auto g = make_bipartite(10, 10, 0.4);

        auto rm = random_maximal_matching(g);
        auto bm = blossom_matching(g);
        auto am = approximate_max_matching(g);

        std::cout << "  Random maximal: " << rm.size() << "\n";
        std::cout << "  Maximum (aug):  " << bm.size() << "\n";
        std::cout << "  1/2-approx:     " << am.size() << "\n";
        std::cout << "  Ratio (max/approx): "
                  << (am.size() > 0 ? static_cast<double>(am.size()) / bm.size() : 0) << "\n\n";
    }

    // Sparse bipartite: ensure connectivity
    {
        std::cout << "Sparse bipartite (8+8, p=0.2):\n";
        auto g = make_bipartite(8, 8, 0.2);

        auto rm = random_maximal_matching(g);
        auto bm = blossom_matching(g);

        std::cout << "  Random maximal: " << rm.size() << "\n";
        std::cout << "  Maximum:        " << bm.size() << "\n\n";
    }

    // Guaranteed perfect matching: random permutation
    {
        std::cout << "Bipartite with hidden perfect matching (10+10):\n";
        auto g = make_bipartite(10, 10, 0.2);

        // Add guaranteed perfect matching
        std::vector<int> perm(10);
        std::iota(perm.begin(), perm.end(), 0);
        rng().shuffle(perm);
        for (int i = 0; i < 10; i++)
            g.add_edge(i, 10 + perm[i]);

        auto bm = blossom_matching(g);
        std::cout << "  Maximum: " << bm.size() << " (expected 10)\n";
    }

    return 0;
}
