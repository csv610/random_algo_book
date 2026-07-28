// example_fermat_primality.cpp
//
// Fermat Primality Test
// =======================
// Based on Fermat's Little Theorem: if p is prime and gcd(a, p) = 1,
// then a^(p-1) ≡ 1 (mod p).
//
// Test: for random a in [2, n-2], compute a^(n-1) mod n.
//   - If the result ≠ 1, n is definitely composite.
//   - If the result = 1 for all tested a, n is "probably prime".
//
// Limitation: Carmichael numbers are composite numbers that pass the
// Fermat test for all bases coprime to them. The smallest is 561 = 3×11×17.
// This is why Miller-Rabin is preferred over Fermat in practice.

#include "ral/crypto.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Fermat Primality Test Demo\n";
    std::cout << "========================================================\n\n";

    // --- Basic test ---
    std::cout << "--- Fermat Test Results ---\n";
    std::vector<long long> numbers = {2, 3, 4, 17, 561, 997, 1009, 104729};
    for (long long n : numbers) {
        bool result = fermat_primality_test(n, 20);
        std::cout << "  n = " << n << " → "
                  << (result ? "probably prime" : "composite") << "\n";
    }

    // --- Carmichael number failure case ---
    std::cout << "\n--- Carmichael Number: 561 ---\n";
    std::cout << "  561 = 3 × 11 × 17 (composite)\n";
    std::cout << "  But Fermat test often passes it!\n\n";

    // Show why: compute a^(560) mod 561 for several bases
    std::cout << "  Testing a^(n-1) mod n for a = 2..30:\n";
    int pass_count = 0;
    for (long long a = 2; a <= 30; ++a) {
        long long result = mod_pow(a, 560, 561);
        bool passed = (result == 1);
        if (passed) ++pass_count;
        if (a <= 10) {
            std::cout << "    a=" << a << ": " << a << "^560 mod 561 = "
                      << result << (passed ? "  (pass)" : "  (FAIL → composite)") << "\n";
        }
    }
    std::cout << "    ...\n";
    std::cout << "  " << pass_count << " out of 29 bases passed (Fermat says 'probably prime')\n";
    std::cout << "  This is why Carmichael numbers are dangerous!\n";

    // --- Miller-Rabin catches it ---
    std::cout << "\n--- Miller-Rabin Correctly Catches 561 ---\n";
    std::cout << "  miller_rabin(561) = "
              << (miller_rabin(561, 20) ? "prime" : "composite") << "\n";

    // --- Other Carmichael numbers ---
    std::cout << "\n--- Other Carmichael Numbers ---\n";
    std::vector<long long> carmichaels = {561, 1105, 1729, 2465, 2821, 6601};
    for (long long n : carmichaels) {
        bool ft = fermat_primality_test(n, 50);
        bool mr = miller_rabin(n, 50);
        std::cout << "  " << n << ": Fermat="
                  << (ft ? "prime" : "composite")
                  << ", Miller-Rabin="
                  << (mr ? "prime" : "composite")
                  << (ft && !mr ? "  ← Fermat fooled!" : "") << "\n";
    }

    // --- When Fermat works fine ---
    std::cout << "\n--- Fermat Works for True Primes ---\n";
    std::vector<long long> primes = {97, 101, 997, 1009, 104729};
    for (long long n : primes) {
        bool result = fermat_primality_test(n, 20);
        std::cout << "  " << n << " → " << (result ? "prime" : "composite") << "\n";
    }

    return 0;
}
