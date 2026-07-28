// example_monte_carlo_min_cut.cpp
// Monte Carlo estimation of the minimum cut in a graph using
// randomized edge contraction (Karger's algorithm idea).
//
// This is a Monte Carlo algorithm: it runs in deterministic polynomial time,
// but the answer may be wrong. However, by repeating and taking the minimum,
// the error probability can be made exponentially small.
//
// The algorithm randomly permutes edges and adds them one by one (via
// union-find) until only 2 components remain. The crossing edges form a cut.
// Pr[correct] >= 1/n^2 per trial for simple graphs.

#include "ral/las_vegas_monte_carlo.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <climits>

using namespace ral;

int main() {
    std::cout << "=== Monte Carlo: Randomized Min-Cut ===\n\n";

    // Build a graph with known min-cut
    // Graph: two triangles joined by a single bridge edge
    // Vertices: 0-1-2 (triangle), 3-4-5 (triangle), edge 2-3 (bridge)
    // Min-cut = 1 (cut the bridge)

    int n = 6;
    std::vector<std::pair<int, int>> edges = {
        {0, 1}, {0, 2},  // triangle 1
        {1, 2},
        {3, 4}, {3, 5},  // triangle 2
        {4, 5},
        {2, 3}            // bridge
    };

    std::cout << "Graph: two triangles joined by a bridge\n";
    std::cout << "Vertices: " << n << ", Edges: " << edges.size() << "\n";
    std::cout << "True min-cut: 1 (the bridge edge 2-3)\n\n";

    // Run with increasing numbers of trials
    std::cout << std::setw(12) << "trials"
              << std::setw(14) << "min-cut found"
              << "\n";
    std::cout << std::string(26, '-') << "\n";

    for (int trials : {1, 2, 5, 10, 20, 50, 100}) {
        int result = randomized_min_cut_monte_carlo(edges, n, trials);
        std::cout << std::setw(12) << trials
                  << std::setw(14) << result
                  << "\n";
    }

    // Larger example: grid-like graph
    std::cout << "\nLarger graph: 4x4 grid (16 vertices)\n";
    {
        int grid_n = 16;
        std::vector<std::pair<int, int>> grid_edges;
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                int v = r * 4 + c;
                if (c < 3) grid_edges.push_back({v, v + 1});
                if (r < 3) grid_edges.push_back({v, v + 4});
            }
        }
        std::cout << "  Vertices: " << grid_n << ", Edges: " << grid_edges.size() << "\n";

        std::cout << std::setw(12) << "trials"
                  << std::setw(14) << "min-cut found"
                  << "\n";
        std::cout << std::string(26, '-') << "\n";

        for (int trials : {1, 5, 10, 50, 100}) {
            int result = randomized_min_cut_monte_carlo(grid_edges, grid_n, trials);
            std::cout << std::setw(12) << trials
                      << std::setw(14) << result
                      << "\n";
        }
    }

    std::cout << "\nMonte Carlo properties:\n";
    std::cout << "  - Running time: deterministic O(m * trials)\n";
    std::cout << "  - Error probability: decreases exponentially with trials\n";
    std::cout << "  - Answer is always >= true min-cut (one-sided error)\n";
    std::cout << "  - Repeating and taking minimum reduces error probability\n";

    return 0;
}
