// example_euler_totient.cpp
//
// Euler's Totient Function φ(n)
// ================================
// φ(n) counts the number of integers in {1, 2, ..., n} that are coprime to n.
//
// Formula: φ(n) = n * Π(1 - 1/p) for all distinct prime factors p of n.
//
// Examples:
//   φ(1)  = 1   (only 1 is coprime to 1)
//   φ(12) = 4   (1, 5, 7, 11 are coprime to 12)
//   φ(36) = 12  (1,5,7,11,13,17,19,23,25,29,31,35)
//
// Practical uses:
//   - RSA: φ(n) where n=p*q determines the key space
//   - Euler's theorem: a^φ(n) ≡ 1 (mod n) if gcd(a,n)=1
//   - Multiplicative group structure modulo n

#include "ral/number_theory.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Euler's Totient Function Demo\n";
    std::cout << "========================================================\n\n";

    // --- Basic totient values ---
    std::cout << "--- Totient Values ---\n";
    std::vector<long long> nums = {1, 2, 3, 6, 10, 12, 36, 100, 3233};
    for (long long n : nums) {
        std::cout << "  φ(" << n << ") = " << euler_totient(n) << "\n";
    }

    // --- Listing coprime numbers ---
    std::cout << "\n--- Coprime Integers for n = 12 ---\n";
    {
        long long n = 12;
        std::cout << "  Numbers coprime to " << n << ": ";
        int count = 0;
        for (long long i = 1; i <= n; ++i) {
            if (gcd(i, n) == 1) {
                std::cout << i;
                if (i < n) std::cout << ", ";
                ++count;
            }
        }
        std::cout << "\n  Count: " << count << " (matches φ(12) = " << euler_totient(n) << ")\n";
    }

    // --- Euler's Theorem: a^φ(n) ≡ 1 (mod n) ---
    std::cout << "\n--- Euler's Theorem: a^φ(n) ≡ 1 (mod n) ---\n";
    std::cout << "  (when gcd(a, n) = 1)\n\n";
    {
        long long n = 15;
        long long phi_n = euler_totient(n);
        std::cout << "  n = " << n << ", φ(n) = " << phi_n << "\n";
        for (long long a = 2; a <= 14; ++a) {
            if (gcd(a, n) != 1) continue;
            long long result = mod_pow(a, phi_n, n);
            std::cout << "  " << a << "^" << phi_n << " mod " << n << " = " << result
                      << (result == 1 ? "  OK" : "  FAIL") << "\n";
        }
    }

    // --- RSA connection ---
    std::cout << "\n--- RSA Connection ---\n";
    {
        long long p = 61, q = 53;
        long long n = p * q;
        long long phi_n = (p - 1) * (q - 1);
        std::cout << "  p = " << p << ", q = " << q << "\n";
        std::cout << "  n = p * q = " << n << "\n";
        std::cout << "  φ(n) = (p-1)(q-1) = " << phi_n << "\n";
        std::cout << "  φ(n) = euler_totient(n) = " << euler_totient(n) << "\n";
        std::cout << "  (both methods agree)\n";
    }

    // --- Totient table ---
    std::cout << "\n--- Totient Table: φ(n) for n = 1..20 ---\n";
    std::cout << "  ";
    for (long long n = 1; n <= 20; ++n) {
        std::cout << "φ(" << n << ")=" << euler_totient(n);
        if (n < 20) std::cout << ", ";
    }
    std::cout << "\n";

    // --- Ratio φ(n)/n ---
    std::cout << "\n--- Ratio φ(n)/n (density of coprimes) ---\n";
    for (long long n : {6, 12, 30, 210, 2310}) {
        std::cout << "  φ(" << n << ")/" << n << " = " << euler_totient(n)
                  << "/" << n << " = " << static_cast<double>(euler_totient(n)) / n << "\n";
    }

    return 0;
}
