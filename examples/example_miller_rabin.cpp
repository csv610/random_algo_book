// example_miller_rabin.cpp
//
// Miller-Rabin Primality Test
// =============================
// A probabilistic test that determines if n is composite or probably prime.
//
// Algorithm:
//   1. Write n - 1 = 2^r * d (factor out powers of 2)
//   2. For a random base a in [2, n-2]:
//      a. Compute x = a^d mod n
//      b. If x == 1 or x == n-1, this base passes → next base
//      c. Square x up to r-1 times. If any square equals n-1 → pass
//      d. If none equal n-1 → n is definitely composite
//   3. If all k bases pass → n is probably prime (error probability ≤ 4^(-k))
//
// With k=20 rounds, the probability of misclassifying a composite is < 10^(-12).
// This is the standard primality test used in practice (e.g., for RSA keygen).

#include "ral/crypto.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Miller-Rabin Primality Test Demo\n";
    std::cout << "========================================================\n\n";

    // --- Test various numbers ---
    std::cout << "--- Miller-Rabin Results ---\n";
    std::vector<long long> numbers = {
        2, 3, 4, 17, 561, 997, 1009, 104729, 1000003, 1000033
    };

    for (long long n : numbers) {
        bool result = miller_rabin(n, 20);
        std::cout << "  n = " << n << " → "
                  << (result ? "probably prime" : "composite") << "\n";
    }

    // --- Show n-1 = 2^r * d decomposition ---
    std::cout << "\n--- n-1 Decomposition: n-1 = 2^r * d ---\n";
    std::vector<long long> decomp = {97, 1009, 561, 104729};
    for (long long n : decomp) {
        long long d = n - 1;
        int r = 0;
        while (d % 2 == 0) {
            d /= 2;
            ++r;
        }
        std::cout << "  " << n << "-1 = " << (n - 1) << " = 2^" << r << " * " << d << "\n";
    }

    // --- Compare with trial division ---
    std::cout << "\n--- Comparison with Trial Division ---\n";
    std::cout << "  Testing numbers 900-1000:\n";
    int mr_prime = 0, td_prime = 0;
    for (long long n = 900; n <= 1000; ++n) {
        if (miller_rabin(n, 20)) ++mr_prime;
        if (is_prime_trial(n)) ++td_prime;
    }
    std::cout << "  Miller-Rabin found " << mr_prime << " primes\n";
    std::cout << "  Trial division found " << td_prime << " primes\n";
    std::cout << "  " << (mr_prime == td_prime ? "Agree!" : "Discrepancy!") << "\n";

    // --- Large prime verification ---
    std::cout << "\n--- Large Prime Verification ---\n";
    std::vector<long long> large = {1000000007, 1000000009, 999999937};
    for (long long n : large) {
        std::cout << "  " << n << " → "
                  << (miller_rabin(n, 30) ? "probably prime" : "composite") << "\n";
    }

    // --- Carmichael number ---
    std::cout << "\n--- Carmichael Number Detection ---\n";
    std::cout << "  561 = 3 * 11 * 17 is a Carmichael number:\n";
    std::cout << "  Miller-Rabin: " << (miller_rabin(561, 20) ? "prime" : "composite") << "\n";
    std::cout << "  Trial division: " << (is_prime_trial(561) ? "prime" : "composite") << "\n";
    std::cout << "  (Miller-Rabin correctly identifies it as composite)\n";

    return 0;
}
