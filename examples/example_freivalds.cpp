// =============================================================================
// Freivalds' Algorithm for Matrix Product Verification
//
// Given n x n matrices A, B, C, verify whether A*B = C in O(n^2) time
// (vs O(n^3) for naive multiplication), with one-sided error.
//
// Algorithm: pick a random vector r in {0,1}^n. Check whether
//   A*(B*r) == C*r.
// If AB != C, then Pr[check passes] <= 1/2, so t rounds gives
// error probability <= 2^{-t}.
//
// This is a classic BPP algorithm for an algebraic decision problem.
// =============================================================================

#include "ral/freivalds.h"
#include <iostream>
#include <vector>
#include <random>

namespace ral {

inline void demonstrate_freivalds_detail() {
    std::mt19937 rng(42);
    const int ROUNDS = 20;

    std::cout << "=== Freivalds' Matrix Product Verification ===\n";
    std::cout << "Rounds: " << ROUNDS << "  (error prob <= 2^-" << ROUNDS << ")\n\n";

    // --- Test 1: 2x2 correct product ---
    std::vector<std::vector<long long>> A = {{1, 2}, {3, 4}};
    std::vector<std::vector<long long>> B = {{5, 6}, {7, 8}};
    std::vector<std::vector<long long>> C = {{19, 22}, {43, 50}};

    std::cout << "A = [1 2; 3 4]\n";
    std::cout << "B = [5 6; 7 8]\n";
    std::cout << "C = [19 22; 43 50]  (correct A*B)\n";
    bool ok = freivalds_verify(A, B, C, ROUNDS, rng);
    std::cout << "Result: " << (ok ? "PASS (AB=C confirmed)" : "FAIL") << "\n\n";

    // --- Test 2: 2x2 incorrect product ---
    std::vector<std::vector<long long>> C_bad = {{20, 22}, {43, 50}};
    std::cout << "C' = [20 22; 43 50]  (C'[0][0] wrong)\n";
    ok = freivalds_verify(A, B, C_bad, ROUNDS, rng);
    std::cout << "Result: " << (ok ? "PASS (missed error!)" : "FAIL (AB != C' detected)") << "\n\n";

    // --- Test 3: 4x4 matrices ---
    std::vector<std::vector<long long>> A4 = {
        {1, 0, 2, 1}, {0, 3, 1, 0}, {2, 1, 0, 1}, {1, 0, 1, 2}
    };
    std::vector<std::vector<long long>> B4 = {
        {1, 1, 0, 2}, {0, 2, 1, 0}, {1, 0, 3, 1}, {2, 1, 0, 1}
    };

    // Compute correct C4 = A4 * B4
    std::vector<std::vector<long long>> C4(4, std::vector<long long>(4, 0));
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                C4[i][j] += A4[i][k] * B4[k][j];
            }
        }
    }

    std::cout << "4x4 correct product:\n";
    ok = freivalds_verify(A4, B4, C4, ROUNDS, rng);
    std::cout << "Result: " << (ok ? "PASS" : "FAIL") << "\n";

    // Introduce a subtle error
    C4[2][3] = 999;
    std::cout << "\n4x4 with C[2][3] changed to 999:\n";
    ok = freivalds_verify(A4, B4, C4, ROUNDS, rng);
    std::cout << "Result: " << (ok ? "PASS (missed!)" : "FAIL (error detected)") << "\n\n";

    // --- Test 4: Single-entry error detection ---
    std::cout << "Detection rate for single-entry errors (100 trials each):\n";
    // Restore correct C4
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j) {
            C4[i][j] = 0;
            for (int k = 0; k < 4; ++k) C4[i][j] += A4[i][k] * B4[k][j];
        }

    for (int pos = 0; pos < 4; ++pos) {
        std::vector<std::vector<long long>> Ctmp = C4;
        Ctmp[1][pos] += 1;  // flip one entry
        int detected = 0;
        for (int trial = 0; trial < 100; ++trial) {
            std::mt19937 trial_rng(trial);
            if (!freivalds_verify(A4, B4, Ctmp, 1, trial_rng)) detected++;
        }
        std::cout << "  C[1][" << pos << "] += 1: detected " << detected << "/100 times\n";
    }
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_freivalds_detail();
    return 0;
}
