// example_solovay_strassen.cpp
//
// Solovay-Strassen Primality Test
// =================================
// A probabilistic test based on the Euler criterion and Jacobi symbol.
//
// For an odd n and random a in [2, n-2]:
//   Compute euler = a^((n-1)/2) mod n
//   Compute js = jacobi_symbol(a, n)
//   If euler ≠ js (mod n), then n is definitely composite.
//
// If n is prime, then a^((n-1)/2) ≡ (a/n) (mod n) for all a (Euler's criterion).
// If n is composite, at least 50% of bases a will reveal this.
//
// With k rounds, error probability ≤ 2^(-k).

#include "ral/crypto.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Solovay-Strassen Primality Test Demo\n";
    std::cout << "========================================================\n\n";

    // --- Test various numbers ---
    std::cout << "--- Solovay-Strassen Results ---\n";
    std::vector<long long> numbers = {
        2, 3, 4, 17, 561, 997, 1009, 104729, 1000003, 1000033
    };

    for (long long n : numbers) {
        bool result = solovay_strassen(n, 20);
        std::cout << "  n = " << n << " → "
                  << (result ? "probably prime" : "composite") << "\n";
    }

    // --- Show Jacobi symbol computation ---
    std::cout << "\n--- Jacobi Symbol Values (needed by Solovay-Strassen) ---\n";
    std::cout << "  For n = 15 (= 3 × 5):\n";
    for (long long a = 1; a < 15; ++a) {
        int js = jacobi_symbol(a, 15);
        long long euler = mod_pow(a, 7, 15);
        std::cout << "    a=" << a << ": (a/15)=" << js
                  << ", a^7 mod 15=" << euler
                  << (euler == (js + 15) % 15 ? "  match" : "  MISMATCH (composite!)") << "\n";
    }

    // --- Comparison with other tests ---
    std::cout << "\n--- All Primality Tests Compared ---\n";
    std::vector<long long> candidates = {2, 17, 561, 997, 1009, 104729};
    std::cout << "  n      | Solovay-Str | Miller-Rabin | Fermat | Trial\n";
    std::cout << "  -------+-------------+--------------+--------+------\n";

    for (long long n : candidates) {
        bool ss = solovay_strassen(n, 20);
        bool mr = miller_rabin(n, 20);
        bool ft = fermat_primality_test(n, 20);
        bool td = is_prime_trial(n);
        std::cout << "  " << n << "  |     "
                  << (ss ? "prime" : "comp  ") << "    |      "
                  << (mr ? "prime" : "comp  ") << "     |  "
                  << (ft ? "prime" : "comp  ") << " |  "
                  << (td ? "prime" : "comp") << "\n";
    }

    return 0;
}
