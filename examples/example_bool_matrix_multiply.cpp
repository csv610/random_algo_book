/*
 * Boolean Matrix Multiplication for Graphs
 *
 * Given adjacency matrices A and B of directed/undirected graphs,
 * computes (A x B)[i][j] = OR_k(A[i][k] AND B[k][j]).
 *
 * In graph terms: (A^k)[i][j] is true iff there is a walk of length k
 * from i to j. Used as a subroutine in Seidel's APSP algorithm.
 *
 * Time: O(n^3) naive, can be improved with fast matrix multiply.
 */

#include "ral/apsp.h"
#include <iostream>
#include <iomanip>

using namespace ral;

void print_bool_matrix(const BoolMatrix& M, const std::string& label, int max_n = 10) {
    int n = static_cast<int>(M.size());
    n = std::min(n, max_n);
    std::cout << label << ":\n";
    for (int i = 0; i < n; i++) {
        std::cout << "  ";
        for (int j = 0; j < n; j++)
            std::cout << (M[i][j] ? " 1" : " .");
        std::cout << "\n";
    }
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Boolean Matrix Multiplication\n";
    std::cout << "========================================================\n\n";

    // Small example: path graph 0-1-2-3
    constexpr int n = 4;
    BoolMatrix A(n, std::vector<bool>(n, false));

    // Adjacency: 0-1, 1-2, 2-3
    A[0][1] = A[1][0] = true;
    A[1][2] = A[2][1] = true;
    A[2][3] = A[3][2] = true;

    print_bool_matrix(A, "Adjacency matrix A (path 0-1-2-3)");

    // A^2: pairs with a walk of length 2
    auto A2 = bool_matrix_multiply(A, A);
    print_bool_matrix(A2, "\nA^2 (walks of length 2)");

    // A^3: pairs with a walk of length 3
    auto A3 = bool_matrix_multiply(A2, A);
    print_bool_matrix(A3, "\nA^3 (walks of length 3)");

    // Interpretation
    std::cout << "\nInterpretation:\n";
    std::cout << "  A^2[0][2] = " << (A2[0][2] ? "true" : "false")
              << " (path 0-1-2 has length 2)\n";
    std::cout << "  A^3[0][3] = " << (A3[0][3] ? "true" : "false")
              << " (path 0-1-2-3 has length 3)\n";
    std::cout << "  A^2[0][3] = " << (A2[0][3] ? "true" : "false")
              << " (no walk of length 2 from 0 to 3)\n";

    // Larger random example
    std::cout << "\n--- Random graph (n=10) boolean powers ---\n";
    auto rand_adj = random_unweighted_graph(10, 0.4);
    auto rand_A2 = bool_matrix_multiply(rand_adj, rand_adj);
    auto rand_A3 = bool_matrix_multiply(rand_A2, rand_adj);

    int edges = 0, walks2 = 0, walks3 = 0;
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++) {
            if (rand_adj[i][j]) edges++;
            if (rand_A2[i][j]) walks2++;
            if (rand_A3[i][j]) walks3++;
        }
    std::cout << "  Adjacency (directed pairs): " << edges / 2 << " edges\n";
    std::cout << "  A^2 pairs: " << walks2 / 2 << " (walks of length 2)\n";
    std::cout << "  A^3 pairs: " << walks3 / 2 << " (walks of length 3)\n";

    return 0;
}
