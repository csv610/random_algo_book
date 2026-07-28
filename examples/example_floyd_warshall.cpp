/*
 * Floyd-Warshall All-Pairs Shortest Paths
 *
 * The classic O(n^3) dynamic programming algorithm for APSP.
 * For each intermediate vertex k, update all pairs (i,j) via:
 *   D[i][j] = min(D[i][j], D[i][k] + D[k][j])
 *
 * Handles positive and negative edge weights (no negative cycles).
 * Serves as ground truth for randomized APSP algorithms.
 */

#include "ral/apsp.h"
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace ral;

void print_matrix(const Matrix& D, int max_n = 10) {
    int n = static_cast<int>(D.size());
    n = std::min(n, max_n);
    std::cout << "    ";
    for (int j = 0; j < n; j++)
        std::cout << std::setw(6) << j;
    std::cout << "\n";
    for (int i = 0; i < n; i++) {
        std::cout << "  " << std::setw(2) << i << " ";
        for (int j = 0; j < n; j++) {
            if (D[i][j] >= INF / 2)
                std::cout << std::setw(6) << "-";
            else
                std::cout << std::setw(6) << static_cast<int>(D[i][j]);
        }
        std::cout << "\n";
    }
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Floyd-Warshall All-Pairs Shortest Paths\n";
    std::cout << "========================================================\n\n";

    // Small graph with known distances
    constexpr int n = 5;
    Matrix W(n, std::vector<double>(n, INF));
    for (int i = 0; i < n; i++) W[i][i] = 0;

    auto add = [&](int u, int v, double w) { W[u][v] = W[v][u] = w; };
    add(0, 1, 10); add(0, 2, 3); add(1, 2, 1);
    add(1, 3, 2); add(2, 3, 8); add(2, 4, 4);
    add(3, 4, 7); add(1, 4, 6);

    std::cout << "Input adjacency matrix (5 vertices):\n";
    print_matrix(W);
    std::cout << "\n";

    auto D = floyd_warshall(W);

    std::cout << "All-pairs shortest distances:\n";
    print_matrix(D);

    // Verify some known distances
    std::cout << "\nSpot checks:\n";
    std::cout << "  d(0,1) = " << D[0][1] << " (expected 4 via 0->2->1)\n";
    std::cout << "  d(0,3) = " << D[0][3] << " (expected 6 via 0->2->1->3)\n";
    std::cout << "  d(0,4) = " << D[0][4] << " (expected 7 via 0->2->4)\n";

    // Verify against min-plus repeated squaring
    auto mps = apsp_repeated_squaring(W);
    std::cout << "\nMin-plus squaring matches: "
              << (verify_apsp(D, mps) ? "YES" : "NO") << "\n";

    return 0;
}
