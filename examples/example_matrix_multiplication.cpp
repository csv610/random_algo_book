// =============================================================================
// Freivalds vs Naive Matrix Multiplication: Comparison
//
// Compare the runtime of:
//   1. Naive matrix multiplication: O(n^3)
//   2. Freivalds' verification:     O(n^2) per round
//
// For verification of A*B = C, Freivalds is dramatically faster than
// recomputing the product. This example times both approaches and shows
// the practical speedup, then verifies correctness of the results.
// =============================================================================

#include "ral/freivalds.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>

namespace ral {

using Matrix = std::vector<std::vector<long long>>;

inline Matrix naive_multiply(const Matrix& A, const Matrix& B) {
    int n = static_cast<int>(A.size());
    Matrix C(n, std::vector<long long>(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < n; ++k)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

inline Matrix make_random_matrix(int n, std::mt19937& rng, int val_range = 100) {
    std::uniform_int_distribution<long long> dist(-val_range, val_range);
    Matrix M(n, std::vector<long long>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            M[i][j] = dist(rng);
    return M;
}

inline void demonstrate_comparison() {
    std::mt19937 rng(42);
    const int ROUNDS = 20;

    std::cout << "=== Freivalds vs Naive Matrix Multiply: Timing ===\n\n";

    std::vector<int> sizes = {10, 20, 50, 100, 200, 300};

    std::cout << std::left
              << std::setw(8) << "n"
              << std::setw(18) << "Naive multiply"
              << std::setw(18) << "Freivalds verif"
              << std::setw(14) << "Speedup"
              << std::setw(12) << "Correct?" << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (int n : sizes) {
        Matrix A = make_random_matrix(n, rng);
        Matrix B = make_random_matrix(n, rng);

        // Time naive multiplication
        auto t0 = std::chrono::high_resolution_clock::now();
        Matrix C = naive_multiply(A, B);
        auto t1 = std::chrono::high_resolution_clock::now();
        double naive_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        // Time Freivalds verification (correct case)
        t0 = std::chrono::high_resolution_clock::now();
        bool correct = freivalds_verify(A, B, C, ROUNDS, rng);
        t1 = std::chrono::high_resolution_clock::now();
        double freivalds_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        // Also time Freivalds on wrong matrix
        Matrix C_wrong = C;
        C_wrong[0][0] += 1;
        t0 = std::chrono::high_resolution_clock::now();
        bool wrong = freivalds_verify(A, B, C_wrong, ROUNDS, rng);
        t1 = std::chrono::high_resolution_clock::now();
        double freivalds_wrong_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        double speedup = (naive_ms > 0.001) ? naive_ms / freivalds_ms : 0;

        std::cout << std::setw(8) << n
                  << std::setw(18) << std::fixed << std::setprecision(3) << naive_ms << " ms"
                  << std::setw(18) << std::fixed << std::setprecision(3) << freivalds_ms << " ms"
                  << std::setw(14) << std::fixed << std::setprecision(1) << speedup << "x"
                  << std::setw(12) << (correct ? "OK" : "FAIL") << "\n";
    }

    // Detailed comparison for n=100
    std::cout << "\n--- Detailed n=100 Example ---\n";
    int n = 100;
    Matrix A = make_random_matrix(n, rng);
    Matrix B = make_random_matrix(n, rng);

    auto t_start = std::chrono::high_resolution_clock::now();
    Matrix C = naive_multiply(A, B);
    auto t_end = std::chrono::high_resolution_clock::now();
    double naive_time = std::chrono::duration<double, std::milli>(t_end - t_start).count();

    std::cout << "  Naive A*B computed in " << std::fixed << std::setprecision(2) << naive_time << " ms\n";

    // Introduce errors at different positions
    std::cout << "\n  Freivalds detection of single-entry errors (20 rounds):\n";
    for (int pos = 0; pos < 5; ++pos) {
        Matrix C_err = C;
        C_err[pos][pos] += 1;  // diagonal error
        t_start = std::chrono::high_resolution_clock::now();
        bool detected = !freivalds_verify(A, B, C_err, ROUNDS, rng);
        t_end = std::chrono::high_resolution_clock::now();
        double verif_time = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        std::cout << "    C[" << pos << "][" << pos << "] += 1: detected="
                  << (detected ? "YES" : "NO")
                  << "  time=" << std::fixed << std::setprecision(2) << verif_time << " ms\n";
    }

    // Cost analysis
    std::cout << "\n--- Complexity Analysis ---\n";
    std::cout << "  Naive multiply:    O(n^3) arithmetic operations\n";
    std::cout << "  Freivalds verify:  O(n^2) per round, " << ROUNDS << " rounds = O(" << ROUNDS << "n^2)\n";
    std::cout << "  For n=100: naive ~1,000,000 ops, freivalds ~200,000 ops\n";
    std::cout << "  For n=1000: naive ~1,000,000,000 ops, freivalds ~20,000,000 ops\n";
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_comparison();
    return 0;
}
