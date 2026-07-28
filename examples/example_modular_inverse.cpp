// example_modular_inverse.cpp
//
// Modular Inverse
// ================
// Given integers a and m, find x such that:  a * x ≡ 1 (mod m)
// This inverse exists if and only if gcd(a, m) = 1.
//
// Computed using the Extended Euclidean Algorithm: if extended_gcd(a, m)
// returns (g, x, y) with g = 1, then x (reduced mod m) is the inverse.
//
// Practical uses:
//   - RSA: computing private key d = e^(-1) mod phi(n)
//   - Solving linear congruences: ax ≡ b (mod m) -> x ≡ b * a^(-1) (mod m)
//   - Fraction arithmetic in modular fields

#include "ral/number_theory.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Modular Inverse Demo\n";
    std::cout << "========================================================\n\n";

    // --- Basic modular inverses ---
    std::cout << "--- Modular Inverses ---\n";
    struct InvPair { long long a, m; };
    std::vector<InvPair> pairs = {{3, 7}, {17, 43}, {10, 17}, {5, 26}, {7, 31}};

    for (auto [a, m] : pairs) {
        long long inv = mod_inverse(a, m);
        std::cout << "  " << a << "^(-1) mod " << m << " = " << inv
                  << "  (check: " << a << " * " << inv << " mod " << m
                  << " = " << (a * inv) % m << ")\n";
    }

    // --- Solving linear congruences ---
    std::cout << "\n--- Solving Linear Congruences: ax ≡ b (mod m) ---\n";
    std::cout << "  Solution: x ≡ b * a^(-1) (mod m)\n\n";

    struct Congruence { long long a, b, m; };
    std::vector<Congruence> congs = {
        {3, 4, 7},    // 3x ≡ 4 (mod 7)
        {7, 2, 13},   // 7x ≡ 2 (mod 13)
        {5, 1, 17},   // 5x ≡ 1 (mod 17)  -- just computing inverse
    };

    for (auto [a, b, m] : congs) {
        long long inv = mod_inverse(a, m);
        long long x = (b * inv) % m;
        std::cout << "  " << a << "x ≡ " << b << " (mod " << m << ")\n";
        std::cout << "    " << a << "^(-1) mod " << m << " = " << inv << "\n";
        std::cout << "    x ≡ " << b << " * " << inv << " ≡ " << x << " (mod " << m << ")\n";
        std::cout << "    Verify: " << a << " * " << x << " mod " << m
                  << " = " << (a * x) % m << "\n\n";
    }

    // --- RSA key computation ---
    std::cout << "--- RSA Private Key Computation ---\n";
    {
        long long p = 61, q = 53;
        long long n = p * q;
        long long phi_n = (p - 1) * (q - 1);
        long long e = 17;

        long long d = mod_inverse(e, phi_n);
        std::cout << "  p = " << p << ", q = " << q << ", n = " << n << "\n";
        std::cout << "  phi(n) = " << phi_n << "\n";
        std::cout << "  e = " << e << "\n";
        std::cout << "  d = e^(-1) mod phi(n) = " << d << "\n";
        std::cout << "  Verify: " << e << " * " << d << " mod " << phi_n
                  << " = " << (e * d) % phi_n << "\n";
    }

    // --- Non-existent inverse ---
    std::cout << "\n--- Non-Existent Inverse ---\n";
    std::cout << "  gcd(6, 9) = " << gcd(6, 9) << " ≠ 1, so 6 has no inverse mod 9\n";
    try {
        mod_inverse(6, 9);
        std::cout << "  (should not reach here)\n";
    } catch (const std::runtime_error& e) {
        std::cout << "  Caught: " << e.what() << "\n";
    }

    return 0;
}
