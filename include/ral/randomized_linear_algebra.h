#pragma once

#include <vector>
#include <random>
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace ral {

// ============================================================
// Randomized Numerical Linear Algebra (RandNLA)
// Implements Randomized Range Finder and Randomized SVD (rSVD)
// ============================================================
class RandomizedLA {
public:
    using Matrix = std::vector<std::vector<double>>;

    // Helper: Matrix transpose
    static Matrix transpose(const Matrix& A) {
        if (A.empty()) return {};
        int m = static_cast<int>(A.size());
        int n = static_cast<int>(A[0].size());
        Matrix AT(n, std::vector<double>(m));
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                AT[j][i] = A[i][j];
            }
        }
        return AT;
    }

    // Helper: Matrix multiplication C = A * B
    static Matrix multiply(const Matrix& A, const Matrix& B) {
        if (A.empty() || B.empty()) return {};
        int m = static_cast<int>(A.size());
        int n = static_cast<int>(A[0].size());
        int p = static_cast<int>(B[0].size());
        if (static_cast<int>(B.size()) != n) {
            throw std::invalid_argument("Inner matrix dimensions must agree for multiplication.");
        }

        Matrix C(m, std::vector<double>(p, 0.0));
        for (int i = 0; i < m; ++i) {
            for (int k = 0; k < n; ++k) {
                double aik = A[i][k];
                for (int j = 0; j < p; ++j) {
                    C[i][j] += aik * B[k][j];
                }
            }
        }
        return C;
    }

    // Helper: Classical Gram-Schmidt Orthonormalization (QR Decomposition: Q part)
    // Computes an orthonormal matrix Q of size m x r from a matrix Y of size m x r
    static Matrix orthonormalize(const Matrix& Y) {
        if (Y.empty()) return {};
        int m = static_cast<int>(Y.size());
        int r = static_cast<int>(Y[0].size());

        Matrix Q(m, std::vector<double>(r, 0.0));

        for (int j = 0; j < r; ++j) {
            // Copy column j of Y
            std::vector<double> q(m);
            for (int i = 0; i < m; ++i) q[i] = Y[i][j];

            // Orthogonalize against all previous columns of Q
            for (int k = 0; k < j; ++k) {
                double dot = 0.0;
                for (int i = 0; i < m; ++i) dot += Y[i][j] * Q[i][k];
                for (int i = 0; i < m; ++i) q[i] -= dot * Q[i][k];
            }

            // Normalize
            double norm = 0.0;
            for (int i = 0; i < m; ++i) norm += q[i] * q[i];
            norm = std::sqrt(norm);

            if (norm > 1e-9) {
                for (int i = 0; i < m; ++i) Q[i][j] = q[i] / norm;
            } else {
                for (int i = 0; i < m; ++i) Q[i][j] = 0.0;
            }
        }
        return Q;
    }

    // ============================================================
    // Randomized Range Finder
    // Computes an orthonormal matrix Q whose columns approximate
    // the range of A. Q has size m x (k + oversample).
    // ============================================================
    static Matrix randomized_range_finder(
        const Matrix& A, int target_rank, int oversample, std::mt19937& rng) {
        
        if (A.empty()) return {};
        int n = static_cast<int>(A[0].size());
        int r = target_rank + oversample;
        if (r > n) r = n;

        // Step 1: Draw a random Gaussian matrix Omega (n x r)
        std::normal_distribution<double> gaussian(0.0, 1.0);
        Matrix Omega(n, std::vector<double>(r));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < r; ++j) {
                Omega[i][j] = gaussian(rng);
            }
        }

        // Step 2: Form sample matrix Y = A * Omega (m x r)
        Matrix Y = multiply(A, Omega);

        // Step 3: Compute orthonormal columns of Y (QR Decomposition)
        return orthonormalize(Y);
    }

    // ============================================================
    // Power Iteration Range Finder (Alternative for faster decay)
    // Form sample matrix Y = (A * A^T)^q * A * Omega
    // ============================================================
    static Matrix randomized_range_finder_power(
        const Matrix& A, int target_rank, int oversample, int power_iters, std::mt19937& rng) {
        
        if (A.empty()) return {};
        int n = static_cast<int>(A[0].size());
        int r = target_rank + oversample;
        if (r > n) r = n;

        std::normal_distribution<double> gaussian(0.0, 1.0);
        Matrix Omega(n, std::vector<double>(r));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < r; ++j) {
                Omega[i][j] = gaussian(rng);
            }
        }

        Matrix Y = multiply(A, Omega);
        Matrix AT = transpose(A);

        // Power iterations to enforce singular value decay
        for (int iter = 0; iter < power_iters; ++iter) {
            Matrix Q_temp = orthonormalize(Y);
            Matrix Z = multiply(AT, Q_temp);
            Matrix Z_ortho = orthonormalize(Z);
            Y = multiply(A, Z_ortho);
        }

        return orthonormalize(Y);
    }
};

} // namespace ral
