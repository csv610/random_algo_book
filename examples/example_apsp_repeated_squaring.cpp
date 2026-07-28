/*
 * APSP via Repeated Min-Plus Matrix Squaring
 *
 * Computes all-pairs shortest paths by repeated min-plus (tropical)
 * matrix multiplication: D^(2^k) = D^(2^(k-1)) (x) D^(2^(k-1)).
 * After ceil(log2(n)) squarings, D^(2^k)[i][j] gives the shortest
 * path distance between i and j.
 *
 * Time: O(n^3 log n) - each squaring is O(n^3), and we need log n steps.
 * This is asymptotically worse than Floyd-Warshall but demonstrates
 * the algebraic approach to APSP that generalizes to other semirings.
 */

#include "ral/apsp.h"
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  APSP via Repeated Min-Plus Squaring\n";
    std::cout << "========================================================\n\n";

    // Weighted graph: 6 vertices
    constexpr int n = 6;
    Matrix W(n, std::vector<double>(n, INF));
    for (int i = 0; i < n; i++) W[i][i] = 0;

    auto add = [&](int u, int v, double w) { W[u][v] = W[v][u] = w; };
    add(0, 1, 5);  add(0, 2, 10); add(1, 2, 3);
    add(1, 3, 8);  add(2, 4, 2);  add(3, 4, 6);
    add(3, 5, 4);  add(4, 5, 1);

    std::cout << "Graph with " << n << " vertices and weighted edges\n\n";

    // Floyd-Warshall as baseline
    auto fw = floyd_warshall(W);

    // Min-plus repeated squaring
    auto mps = apsp_repeated_squaring(W);

    // Print comparison
    std::cout << "Pair  Floyd-Warshall  Min-Plus Sq\n";
    std::cout << "----  --------------  -----------\n";
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (fw[i][j] < INF / 2) {
                std::cout << "(" << i << "," << j << ")    "
                          << std::setw(10) << static_cast<int>(fw[i][j])
                          << "    " << std::setw(10) << static_cast<int>(mps[i][j]) << "\n";
            }
        }
    }

    std::cout << "\nResults match: " << (verify_apsp(fw, mps) ? "YES" : "NO") << "\n";

    // Show step-by-step squaring for a small example
    std::cout << "\n--- Step-by-step squaring for 4-vertex graph ---\n";
    constexpr int m = 4;
    Matrix W2(m, std::vector<double>(m, INF));
    for (int i = 0; i < m; i++) W2[i][i] = 0;
    W2[0][1] = W2[1][0] = 2;
    W2[1][2] = W2[2][1] = 3;
    W2[2][3] = W2[3][2] = 1;

    Matrix D = W2;
    std::cout << "D^1 (adjacency):\n";
    for (int i = 0; i < m; i++) {
        std::cout << "  ";
        for (int j = 0; j < m; j++)
            std::cout << std::setw(4) << (D[i][j] >= INF / 2 ? -1 : static_cast<int>(D[i][j]));
        std::cout << "\n";
    }

    D = min_plus_product(D, D);
    std::cout << "D^2 (paths of length <= 2):\n";
    for (int i = 0; i < m; i++) {
        std::cout << "  ";
        for (int j = 0; j < m; j++)
            std::cout << std::setw(4) << (D[i][j] >= INF / 2 ? -1 : static_cast<int>(D[i][j]));
        std::cout << "\n";
    }

    D = min_plus_product(D, D);
    std::cout << "D^4 (all shortest paths):\n";
    for (int i = 0; i < m; i++) {
        std::cout << "  ";
        for (int j = 0; j < m; j++)
            std::cout << std::setw(4) << (D[i][j] >= INF / 2 ? -1 : static_cast<int>(D[i][j]));
        std::cout << "\n";
    }

    return 0;
}
