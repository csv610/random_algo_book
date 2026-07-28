/*
 * Seidel's Randomized APSP for Unweighted Undirected Graphs
 *
 * Uses boolean matrix multiplication to compute all-pairs shortest paths
 * in unweighted undirected graphs. The key insight: the distance between
 * vertices i and j equals the smallest k such that (A^k)[i][j] is true,
 * where A is the boolean adjacency matrix.
 *
 * Computes all boolean powers A^1 through A^(n-1) iteratively.
 * Time: O(n^3 log n) via repeated squaring, or O(n^3) if all powers
 * are computed sequentially as done here.
 *
 * Reference: "Fast Matrix Multiplication and Its Applications"
 *            - Raimund Seidel, 1995
 */

#include "ral/apsp.h"
#include <iostream>
#include <iomanip>
#include <queue>
#include <cmath>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Seidel's APSP (Unweighted, Boolean Matrix Multiply)\n";
    std::cout << "========================================================\n\n";

    // Build a small unweighted graph
    constexpr int n = 8;
    BoolMatrix adj(n, std::vector<bool>(n, false));

    // Create a path: 0-1-2-3-4-5-6-7
    for (int i = 0; i < n - 1; i++)
        adj[i][i + 1] = adj[i + 1][i] = true;

    // Add shortcuts: 0-3, 2-6
    adj[0][3] = adj[3][0] = true;
    adj[2][6] = adj[6][2] = true;

    std::cout << "Graph: path 0-1-2-3-4-5-6-7 with shortcuts 0-3 and 2-6\n\n";

    // Ground truth via BFS
    Matrix bfs_dist(n, std::vector<double>(n, INF));
    for (int s = 0; s < n; s++) {
        bfs_dist[s][s] = 0;
        std::vector<bool> visited(n, false);
        visited[s] = true;
        std::queue<int> q;
        q.push(s);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v = 0; v < n; v++)
                if (adj[u][v] && !visited[v]) {
                    visited[v] = true;
                    bfs_dist[s][v] = bfs_dist[s][u] + 1;
                    q.push(v);
                }
        }
    }

    // Run Seidel's algorithm
    auto seidel = seidel_apsp_unweighted(adj);

    // Print distances
    std::cout << "Distances (Seidel):\n";
    for (int i = 0; i < n; i++) {
        std::cout << "  ";
        for (int j = 0; j < n; j++)
            std::cout << std::setw(3) << static_cast<int>(seidel[i][j]);
        std::cout << "\n";
    }

    // Verify against BFS
    std::cout << "\nMatches BFS: " << (verify_apsp(seidel, bfs_dist) ? "YES" : "NO") << "\n";

    // Random graph test
    std::cout << "\nRandom unweighted graph (n=20, p=0.3):\n";
    auto rand_adj = random_unweighted_graph(20, 0.3);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto result = seidel_apsp_unweighted(rand_adj);
    auto t2 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "  Time: " << std::fixed << std::setprecision(1) << ms << " ms\n";

    return 0;
}
