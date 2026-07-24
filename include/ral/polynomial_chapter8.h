#pragma once
#include <vector>
#include <random>
#include <iostream>
#include <cassert>

namespace chapter8 {

// Evaluate polynomial with coefficients coeffs at point x modulo mod
// coeffs[i] is the coefficient of x^i: p(x) = coeffs[0] + coeffs[1]*x + ...
inline long long poly_eval(const std::vector<int>& coeffs, long long x, long long mod) {
    long long result = 0;
    long long xpow = 1;
    for (int i = 0; i < (int)coeffs.size(); i++) {
        result = (result + ((long long)coeffs[i] % mod) * xpow % mod) % mod;
        xpow = xpow * x % mod;
    }
    return result;
}

// Test univariate polynomial identity: p(x) == q(x)?
// Returns true if p and q are likely identical, false if different detected
inline bool polynomial_identity_test(const std::vector<int>& p_coeffs,
                              const std::vector<int>& q_coeffs,
                              int field_size,
                              int rounds,
                              std::mt19937& rng) {
    const long long MOD = 1000000007LL;
    std::uniform_int_distribution<int> dist(1, field_size - 1);

    for (int round = 0; round < rounds; round++) {
        long long x = dist(rng);
        long long px = poly_eval(p_coeffs, x, MOD);
        long long qx = poly_eval(q_coeffs, x, MOD);
        if (px != qx) {
            return false;
        }
    }
    return true;
}

// Compute polynomial degree (highest nonzero coefficient index)
inline int poly_degree(const std::vector<int>& coeffs) {
    for (int i = (int)coeffs.size() - 1; i >= 0; i--) {
        if (coeffs[i] != 0) return i;
    }
    return -1;
}

inline void demonstrate_polynomial() {
    std::cout << "=== Polynomial Identity Testing (Schwartz-Zippel) ===\n\n";

    std::mt19937 rng(42);
    int field_size = 10000;

    std::vector<int> p1 = {1, 2, 1};
    std::vector<int> q1 = {1, 2, 1};

    std::cout << "Test 1: p(x) = x^2 + 2x + 1, q(x) = x^2 + 2x + 1\n";
    bool result = polynomial_identity_test(p1, q1, field_size, 10, rng);
    std::cout << "p == q? " << (result ? "Yes (identical)" : "No (different)")
              << " (expected: Yes)\n\n";

    std::vector<int> p2 = {1, 2, 1};
    std::vector<int> q2 = {1, 3, 1};

    std::cout << "Test 2: p(x) = x^2 + 2x + 1, q(x) = x^2 + 3x + 1\n";
    result = polynomial_identity_test(p2, q2, field_size, 10, rng);
    std::cout << "p == q? " << (result ? "Yes (identical)" : "No (different)")
              << " (expected: No)\n\n";

    std::vector<int> p3 = {0, 0, 1};
    std::vector<int> q3 = {1, 2, 1};

    std::cout << "Test 3: p(x) = x^2, q(x) = x^2 + 2x + 1 (clearly different)\n";
    result = polynomial_identity_test(p3, q3, field_size, 10, rng);
    std::cout << "p == q? " << (result ? "Yes (identical)" : "No (different)")
              << " (expected: No)\n\n";

    std::cout << "Polynomial evaluation:\n";
    std::vector<int> poly = {3, 0, 2, 1};
    long long x_val = 5;
    long long mod = 1000000007LL;
    long long val = poly_eval(poly, x_val, mod);
    std::cout << "p(5) where p(x) = 3 + 2x^2 + x^3 = " << val << "\n";
    std::cout << "Expected: " << (3 + 2 * 25 + 125) << "\n\n";

    std::cout << "Polynomial degrees:\n";
    std::cout << "  deg(3 + 2x^2 + x^3) = " << poly_degree(poly) << " (expected: 3)\n";
    std::cout << "  deg(x^2 + 2x + 1) = " << poly_degree(p1) << " (expected: 2)\n";
    std::cout << "  deg(constant 5) = " << poly_degree({5}) << " (expected: 0)\n";
}

} // namespace chapter8
