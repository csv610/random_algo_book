// example_extended_gcd.cpp
//
// Extended Euclidean Algorithm
// =============================
// Finds integers x, y such that:  a*x + b*y = gcd(a, b)
//
// The standard Euclidean algorithm computes gcd(a,b) by repeated division.
// The extended version additionally tracks the linear combination coefficients.
//
//   Example: extended_gcd(35, 15)
//     gcd = 5,  x = 1,  y = -2
//     Verify: 35 * 1 + 15 * (-2) = 35 - 30 = 5  ✓
//
// Practical use: Computing modular inverses. If gcd(a, m) = 1, then
// the x from extended_gcd(a, m) gives a^(-1) mod m (after reduction).

#include "ral/number_theory.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Extended GCD Algorithm Demo\n";
    std::cout << "========================================================\n\n";

    // --- Basic extended GCD ---
    std::cout << "--- Extended GCD Results ---\n";
    std::vector<std::pair<long long, long long>> pairs = {
        {35, 15}, {30, 20}, {99, 78}, {3, 7}, {355, 113}
    };

    for (auto [a, b] : pairs) {
        auto [g, x, y] = extended_gcd(a, b);
        std::cout << "  " << a << " * (" << x << ") + " << b
                  << " * (" << y << ") = " << g << " = gcd(" << a << ", " << b << ")\n";
    }

    // --- Verification ---
    std::cout << "\n--- Verification ---\n";
    for (auto [a, b] : pairs) {
        auto [g, x, y] = extended_gcd(a, b);
        long long check = a * x + b * y;
        std::cout << "  " << a << "*" << x << " + " << b << "*" << y
                  << " = " << check
                  << (check == g ? "  OK" : "  MISMATCH!") << "\n";
    }

    // --- Modular inverse via extended GCD ---
    std::cout << "\n--- Modular Inverse via Extended GCD ---\n";
    std::cout << "  If gcd(a, m) = 1, then x from extended_gcd(a, m) satisfies\n";
    std::cout << "  a*x ≡ 1 (mod m) after reducing x mod m.\n\n";

    struct InverseTest { long long a, m; };
    std::vector<InverseTest> inv_tests = {{3, 7}, {17, 43}, {10, 17}, {123, 1000}};

    for (auto [a, m] : inv_tests) {
        auto [g, x, y] = extended_gcd(a, m);
        if (g == 1) {
            long long inv = ((x % m) + m) % m;
            std::cout << "  " << a << "^(-1) mod " << m << " = " << inv
                      << "  (check: " << a << " * " << inv << " mod " << m
                      << " = " << (a * inv) % m << ")\n";
        } else {
            std::cout << "  " << a << " has no inverse mod " << m
                      << "  (gcd = " << g << ")\n";
        }
    }

    // --- Solving linear Diophantine equations ---
    std::cout << "\n--- Solving Linear Diophantine: 14x + 21y = gcd(14,21) ---\n";
    {
        auto [g, x, y] = extended_gcd(14, 21);
        std::cout << "  gcd(14, 21) = " << g << "\n";
        std::cout << "  x = " << x << ", y = " << y << "\n";
        std::cout << "  Verify: 14*(" << x << ") + 21*(" << y << ") = "
                  << 14 * x + 21 * y << "\n";

        // General solution: x' = x + k*(21/g), y' = y - k*(14/g)
        std::cout << "  General solution: x = " << x << " + k*" << (21 / g)
                  << ", y = " << y << " - k*" << (14 / g) << "\n";
        std::cout << "  For k=1: x=" << (x + 21 / g) << ", y=" << (y - 14 / g)
                  << "  -> 14*(" << (x + 21 / g) << ") + 21*(" << (y - 14 / g)
                  << ") = " << 14 * (x + 21 / g) + 21 * (y - 14 / g) << "\n";
    }

    return 0;
}
