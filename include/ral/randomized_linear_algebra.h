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

    // ============================================================
    // SVD via classical Jacobi rotations (for small matrices)
    // Decomposes A = U * diag(S) * V^T
    // ============================================================
    struct SVDResult {
        Matrix U;
        std::vector<double> S;
        Matrix V;
    };

    static SVDResult classical_svd(const Matrix& A) {
        if (A.empty()) return {};
        int m = static_cast<int>(A.size());
        int n = static_cast<int>(A[0].size());

        // Compute A^T * A (n x n)
        Matrix AT = transpose(A);
        Matrix ATA = multiply(AT, A);

        // Jacobi eigenvalue algorithm on ATA
        Matrix V = identity_matrix(n);
        for (int sweep = 0; sweep < 100; ++sweep) {
            double off = off_diagonal_norm(ATA);
            if (off < 1e-12) break;

            for (int p = 0; p < n; ++p) {
                for (int q = p + 1; q < n; ++q) {
                    if (std::abs(ATA[p][q]) < 1e-15) continue;
                    jacobi_rotation(ATA, V, p, q, n);
                }
            }
        }

        // Extract singular values and build S
        std::vector<double> S(n);
        for (int i = 0; i < n; ++i) {
            S[i] = std::sqrt(std::max(0.0, ATA[i][i]));
        }

        // Sort in descending order
        std::vector<int> perm(n);
        std::iota(perm.begin(), perm.end(), 0);
        std::sort(perm.begin(), perm.end(), [&](int a, int b) {
            return S[a] > S[b];
        });

        std::vector<double> S_sorted(n);
        Matrix V_sorted(n, std::vector<double>(n));
        for (int i = 0; i < n; ++i) {
            S_sorted[i] = S[perm[i]];
            for (int j = 0; j < n; ++j) {
                V_sorted[j][i] = V[j][perm[i]];
            }
        }

        // Compute U = A * V * diag(1/S) for nonzero singular values
        Matrix U(m, std::vector<double>(n, 0.0));
        Matrix AV = multiply(A, V_sorted);
        for (int j = 0; j < n; ++j) {
            if (S_sorted[j] > 1e-12) {
                for (int i = 0; i < m; ++i) {
                    U[i][j] = AV[i][j] / S_sorted[j];
                }
            }
        }

        // Orthonormalize U columns
        U = orthonormalize_columns(U, n);

        return {U, S_sorted, V_sorted};
    }

    // ============================================================
    // Randomized SVD: A ~ U * diag(S) * V^T
    // Uses randomized range finder + deterministic SVD on the small matrix
    // ============================================================
    static SVDResult randomized_svd(
        const Matrix& A, int target_rank, int oversample, std::mt19937& rng) {

        if (A.empty()) return {};
        int m = static_cast<int>(A.size());
        int n = static_cast<int>(A[0].size());
        int r = std::min({target_rank + oversample, m, n});

        // Step 1: Randomized range finder
        Matrix Q = randomized_range_finder(A, target_rank, oversample, rng);

        // Step 2: Project: B = Q^T * A (r x n)
        Matrix QT = transpose(Q);
        Matrix B = multiply(QT, A);

        // Step 3: Classical SVD on the small matrix B
        Matrix BT = transpose(B);
        Matrix BBT = multiply(B, BT);
        Matrix Uhat = identity_matrix(r);
        for (int sweep = 0; sweep < 50; ++sweep) {
            double off = off_diagonal_norm(BBT);
            if (off < 1e-12) break;
            for (int p = 0; p < r; ++p) {
                for (int q = p + 1; q < r; ++q) {
                    if (std::abs(BBT[p][q]) < 1e-15) continue;
                    jacobi_rotation(BBT, Uhat, p, q, r);
                }
            }
        }

        // Extract singular values
        std::vector<double> S(r);
        for (int i = 0; i < r; ++i) {
            S[i] = std::sqrt(std::max(0.0, BBT[i][i]));
        }

        // Sort descending
        std::vector<int> perm(r);
        std::iota(perm.begin(), perm.end(), 0);
        std::sort(perm.begin(), perm.end(), [&](int a, int b) {
            return S[a] > S[b];
        });

        std::vector<double> S_sorted(r);
        Matrix Uhat_sorted(r, std::vector<double>(r));
        for (int i = 0; i < r; ++i) {
            S_sorted[i] = S[perm[i]];
            for (int j = 0; j < r; ++j) {
                Uhat_sorted[j][i] = Uhat[j][perm[i]];
            }
        }

        // Step 4: Recover U = Q * Uhat
        Matrix U = multiply(Q, Uhat_sorted);

        // Step 5: Recover V from B = Uhat^T * Q^T * A
        // B * Uhat = Q^T * A * Uhat (which is close to S * V^T)
        // V^T = diag(1/S) * Uhat^T * B^T
        Matrix BBT2 = multiply(BT, B);
        Matrix Vhat = identity_matrix(r);
        for (int sweep = 0; sweep < 50; ++sweep) {
            double off = off_diagonal_norm(BBT2);
            if (off < 1e-12) break;
            for (int p = 0; p < r; ++p) {
                for (int q = p + 1; q < r; ++q) {
                    if (std::abs(BBT2[p][q]) < 1e-15) continue;
                    jacobi_rotation(BBT2, Vhat, p, q, r);
                }
            }
        }

        // Sort Vhat to match
        Matrix Vhat_sorted(r, std::vector<double>(r));
        for (int i = 0; i < r; ++i) {
            for (int j = 0; j < r; ++j) {
                Vhat_sorted[j][i] = Vhat[j][perm[i]];
            }
        }

        // V_full = Vhat_sorted^T (r x n truncated)
        // Actually we need V of size n x r
        // Since B = Q^T * A, and B = Uhat * Sigma * Vhat^T
        // Then A ~ Q * Uhat * Sigma * Vhat^T
        // So V = Vhat (which is r x r)
        // For full n x r recovery, we'd need more, but truncated is fine.

        // Recover V from B directly
        Matrix V_recovered(r, std::vector<double>(n, 0.0));
        for (int j = 0; j < r; ++j) {
            if (S_sorted[j] > 1e-12) {
                for (int i = 0; i < n; ++i) {
                    double val = 0.0;
                    for (int k = 0; k < r; ++k) {
                        val += Uhat_sorted[k][j] * B[k][i];
                    }
                    V_recovered[j][i] = val / S_sorted[j];
                }
            }
        }

        // Orthonormalize V rows
        Matrix VT = transpose(V_recovered);
        VT = orthonormalize(VT);
        Matrix V_full = transpose(VT);

        return {U, S_sorted, V_full};
    }

private:
    // Jacobi rotation for eigenvalue decomposition
    static void jacobi_rotation(Matrix& A, Matrix& V, int p, int q, int n) {
        if (std::abs(A[p][q]) < 1e-15) return;

        double tau = (A[q][q] - A[p][p]) / (2.0 * A[p][q]);
        double t = std::copysign(1.0, tau) / (std::abs(tau) + std::sqrt(1.0 + tau * tau));
        double c = 1.0 / std::sqrt(1.0 + t * t);
        double s = t * c;

        // Update A
        for (int i = 0; i < n; ++i) {
            double aip = A[i][p];
            double aiq = A[i][q];
            A[i][p] = c * aip - s * aiq;
            A[i][q] = s * aip + c * aiq;
        }
        for (int j = 0; j < n; ++j) {
            double apj = A[p][j];
            double aqj = A[q][j];
            A[p][j] = c * apj - s * aqj;
            A[q][j] = s * apj + c * aqj;
        }

        // Update V
        for (int i = 0; i < n; ++i) {
            double vip = V[i][p];
            double viq = V[i][q];
            V[i][p] = c * vip - s * viq;
            V[i][q] = s * vip + c * viq;
        }
    }

    static double off_diagonal_norm(const Matrix& A) {
        int n = static_cast<int>(A.size());
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j) sum += A[i][j] * A[i][j];
            }
        }
        return std::sqrt(sum);
    }

    static Matrix identity_matrix(int n) {
        Matrix I(n, std::vector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) I[i][i] = 1.0;
        return I;
    }

    static Matrix orthonormalize_columns(const Matrix& A, int cols) {
        int m = static_cast<int>(A.size());
        Matrix Q(m, std::vector<double>(cols, 0.0));
        for (int j = 0; j < cols; ++j) {
            std::vector<double> q(m);
            for (int i = 0; i < m; ++i) q[i] = A[i][j];
            for (int k = 0; k < j; ++k) {
                double dot = 0.0;
                for (int i = 0; i < m; ++i) dot += A[i][j] * Q[i][k];
                for (int i = 0; i < m; ++i) q[i] -= dot * Q[i][k];
            }
            double norm = 0.0;
            for (int i = 0; i < m; ++i) norm += q[i] * q[i];
            norm = std::sqrt(norm);
            if (norm > 1e-9) {
                for (int i = 0; i < m; ++i) Q[i][j] = q[i] / norm;
            }
        }
        return Q;
    }
};

} // namespace ral
