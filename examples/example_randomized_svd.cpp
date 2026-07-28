// =============================================================================
// Randomized SVD Decomposition
//
// Compute a rank-k approximation A ~ U * diag(S) * V^T in O(m*n*k) time,
// which is much faster than the O(m*n*min(m,n)) classical SVD for large
// matrices with rapidly decaying singular values.
//
// Algorithm:
//   1. Draw random Gaussian matrix Omega (n x (k+p)), p = oversample
//   2. Form Y = A * Omega  (captures the range of A)
//   3. Orthonormalize Y to get Q  (approximate range basis)
//   4. Form small matrix B = Q^T * A  (r x n, r = k+p)
//   5. Compute classical SVD of B: B = Uhat * S * Vhat^T
//   6. Recover U = Q * Uhat
//
// The error satisfies ||A - QQ^T A||_2 <= sigma_{k+1}(A) with high
// probability when oversample p is ~5-10.
// =============================================================================

#include "ral/randomized_linear_algebra.h"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <iomanip>

namespace ral {

using Matrix = std::vector<std::vector<double>>;

inline void print_matrix(const std::string& name, const Matrix& M, int max_rows = 6, int max_cols = 6) {
    int m = static_cast<int>(M.size());
    int n = m > 0 ? static_cast<int>(M[0].size()) : 0;
    int show_r = std::min(m, max_rows);
    int show_c = std::min(n, max_cols);

    std::cout << name << " (" << m << "x" << n << "):\n";
    for (int i = 0; i < show_r; ++i) {
        std::cout << "  ";
        for (int j = 0; j < show_c; ++j) {
            std::cout << std::setw(10) << std::fixed << std::setprecision(4) << M[i][j];
        }
        if (show_c < n) std::cout << "  ...";
        std::cout << "\n";
    }
    if (show_r < m) std::cout << "  ...\n";
}

inline double frobenius_error(const Matrix& A, const Matrix& U, const std::vector<double>& S,
                               const Matrix& V, int k) {
    int m = static_cast<int>(A.size());
    int n = static_cast<int>(A[0].size());
    double error = 0.0;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            double approx = 0.0;
            for (int t = 0; t < k; ++t) {
                approx += S[t] * U[i][t] * V[t][j];
            }
            double diff = A[i][j] - approx;
            error += diff * diff;
        }
    }
    return std::sqrt(error);
}

inline void demonstrate_randomized_svd() {
    std::mt19937 rng(42);

    std::cout << "=== Randomized SVD Decomposition ===\n\n";

    // Create a 10x8 matrix with rapid singular value decay
    // A = sum of rank-1 terms with decreasing weights
    int m = 10, n = 8;
    int true_rank = 3;
    std::normal_distribution<double> gauss(0.0, 1.0);

    Matrix A(m, std::vector<double>(n, 0.0));
    std::cout << "Constructing 10x8 matrix as sum of " << true_rank << " rank-1 terms\n";
    std::cout << "with singular values [10, 5, 1] + noise\n\n";

    for (int k = 0; k < true_rank; ++k) {
        double sigma = 10.0 / (k + 1);
        std::vector<double> u(m), v(n);
        for (int i = 0; i < m; ++i) u[i] = gauss(rng);
        for (int j = 0; j < n; ++j) v[j] = gauss(rng);
        // Normalize
        double nu = 0, nv = 0;
        for (int i = 0; i < m; ++i) nu += u[i] * u[i];
        for (int j = 0; j < n; ++j) nv += v[j] * v[j];
        nu = std::sqrt(nu); nv = std::sqrt(nv);
        for (int i = 0; i < m; ++i) u[i] /= nu;
        for (int j = 0; j < n; ++j) v[j] /= nv;

        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                A[i][j] += sigma * u[i] * v[j];
    }

    print_matrix("A", A, m, n);

    // Classical SVD (exact)
    auto svd_exact = RandomizedLA::classical_svd(A);
    std::cout << "\nClassical SVD singular values: ";
    for (int i = 0; i < std::min(8, (int)svd_exact.S.size()); ++i) {
        std::cout << std::fixed << std::setprecision(4) << svd_exact.S[i] << " ";
    }
    std::cout << "\n";

    // Randomized SVD with different target ranks
    for (int k : {1, 2, 3, 5}) {
        int oversample = 5;
        auto svd_rand = RandomizedLA::randomized_svd(A, k, oversample, rng);
        double error = frobenius_error(A, svd_rand.U, svd_rand.S, svd_rand.V, k);

        std::cout << "\nRandomized SVD (rank=" << k << ", oversample=" << oversample << "):\n";
        std::cout << "  Approx singular values: ";
        for (int i = 0; i < std::min(k + oversample, (int)svd_rand.S.size()); ++i) {
            if (svd_rand.S[i] > 1e-6)
                std::cout << std::fixed << std::setprecision(4) << svd_rand.S[i] << " ";
        }
        std::cout << "\n  Frobenius error ||A - A_k||_F = " << std::fixed << std::setprecision(6) << error << "\n";
    }

    // Range finder demonstration
    std::cout << "\n--- Randomized Range Finder ---\n";
    int target = 2, over = 3;
    auto Q = RandomizedLA::randomized_range_finder(A, target, over, rng);
    print_matrix("Q (range basis)", Q, m, target + over);

    double range_error = 0.0;
    Matrix AQ = RandomizedLA::multiply(A, RandomizedLA::transpose(Q));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < target + over; ++j) {
            double proj = 0.0;
            for (int k = 0; k < m; ++k) proj += AQ[i][j] * AQ[i][j];
            range_error += proj;
        }
    std::cout << "  ||AQ||_F (should be small) = " << std::sqrt(range_error) << "\n";
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_randomized_svd();
    return 0;
}
