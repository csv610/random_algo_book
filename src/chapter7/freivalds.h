#pragma once
#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cassert>

namespace chapter7 {

// Freivalds' algorithm: verify AB = C
// Returns true if verification passes (AB = C likely), false if AB != C detected
// Uses modular arithmetic to avoid overflow for large matrices
inline bool freivalds_verify(const std::vector<std::vector<long long>>& A,
                      const std::vector<std::vector<long long>>& B,
                      const std::vector<std::vector<long long>>& C,
                      int rounds,
                      std::mt19937& rng) {
    int n = (int)A.size();
    assert(n > 0 && (int)A[0].size() == n);
    assert((int)B.size() == n && (int)B[0].size() == n);
    assert((int)C.size() == n && (int)C[0].size() == n);

    const long long MOD = 1000000007LL;
    std::uniform_int_distribution<int> bit_dist(0, 1);

    for (int round = 0; round < rounds; round++) {
        std::vector<long long> r(n);
        for (int i = 0; i < n; i++) {
            r[i] = bit_dist(rng);
        }

        // v = B * r (mod MOD)
        std::vector<long long> v(n, 0);
        for (int i = 0; i < n; i++) {
            long long sum = 0;
            for (int j = 0; j < n; j++) {
                sum = (sum + (B[i][j] % MOD) * r[j]) % MOD;
            }
            v[i] = sum;
        }

        // w = A * v (mod MOD)
        std::vector<long long> w(n, 0);
        for (int i = 0; i < n; i++) {
            long long sum = 0;
            for (int j = 0; j < n; j++) {
                sum = (sum + (A[i][j] % MOD) * v[j]) % MOD;
            }
            w[i] = sum;
        }

        // u = C * r (mod MOD)
        std::vector<long long> u(n, 0);
        for (int i = 0; i < n; i++) {
            long long sum = 0;
            for (int j = 0; j < n; j++) {
                sum = (sum + (C[i][j] % MOD) * r[j]) % MOD;
            }
            u[i] = sum;
        }

        for (int i = 0; i < n; i++) {
            if (((w[i] - u[i]) % MOD + MOD) % MOD != 0) {
                return false;
            }
        }
    }
    return true;
}

inline void demonstrate_freivalds() {
    std::cout << "=== Freivalds' Matrix Multiplication Verification ===\n\n";

    std::vector<std::vector<long long>> A = {{1, 2}, {3, 4}};
    std::vector<std::vector<long long>> B = {{5, 6}, {7, 8}};
    std::vector<std::vector<long long>> C = {{19, 22}, {43, 50}};

    std::cout << "A = [1 2; 3 4], B = [5 6; 7 8]\n";
    std::cout << "C = [19 22; 43 50] (correct product)\n";

    std::mt19937 rng(42);
    bool result = freivalds_verify(A, B, C, 20, rng);
    std::cout << "freivalds_verify(A, B, C) = " << (result ? "true" : "false")
              << " (expected: true)\n\n";

    std::vector<std::vector<long long>> C_wrong = {{20, 22}, {43, 50}};
    std::cout << "C_wrong = [20 22; 43 50] (incorrect product)\n";
    result = freivalds_verify(A, B, C_wrong, 20, rng);
    std::cout << "freivalds_verify(A, B, C_wrong) = " << (result ? "true" : "false")
              << " (expected: false)\n\n";

    std::vector<std::vector<long long>> A3 = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    std::vector<std::vector<long long>> B3 = {{9, 8, 7}, {6, 5, 4}, {3, 2, 1}};
    std::vector<std::vector<long long>> C3 = {{30, 24, 18}, {84, 69, 54}, {138, 114, 90}};
    std::cout << "3x3 correct product:\n";
    result = freivalds_verify(A3, B3, C3, 20, rng);
    std::cout << "freivalds_verify = " << (result ? "true" : "false")
              << " (expected: true)\n";

    C3[1][1] = 70;
    std::cout << "3x3 incorrect product (C[1][1] changed to 70):\n";
    result = freivalds_verify(A3, B3, C3, 20, rng);
    std::cout << "freivalds_verify = " << (result ? "true" : "false")
              << " (expected: false)\n";
}

} // namespace chapter7
