// example_prime_factorization.cpp
// Prime Factorization with Exponents
//
// Uses the PrimePower struct to display factorizations as p1^e1 * p2^e2 * ...
// Built on top of pollard_rho_factorize. Useful for computing Euler's
// totient, number of divisors, and other multiplicative functions.
//
// Compile: g++ -std=c++23 -I../include -o example_prime_factorization example_prime_factorization.cpp

#include <random>
#include <ral/pollard_rho.h>
#include <ral/number_theory.h>
#include <iostream>
#include <cmath>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Prime Factorization with Exponents ===\n\n";

    std::vector<long long> numbers = {
        360LL,          // 2^3 * 3^2 * 5
        1024LL,         // 2^10
        13195LL,        // 5 * 7 * 13 * 29
        600851475143LL, // 71 * 839 * 1471 * 6857
    };

    for (long long n : numbers) {
        auto factors = prime_factorization(n, rng);

        std::cout << n << " = ";
        bool first = true;
        for (auto& [p, e] : factors) {
            if (!first) std::cout << " * ";
            first = false;
            std::cout << p;
            if (e > 1) std::cout << "^" << e;
        }
        std::cout << "\n";

        // Derived quantities
        // Number of divisors = product of (e_i + 1)
        int num_divisors = 1;
        for (auto& [p, e] : factors) num_divisors *= (e + 1);

        // Euler's totient = n * prod(1 - 1/p)
        long long phi = euler_totient(n);

        std::cout << "  Number of divisors: " << num_divisors << "\n";
        std::cout << "  Euler's totient:    " << phi << "\n\n";
    }

    // Show that factorization is consistent
    std::cout << "Verification (product of p^e == n):\n";
    std::vector<long long> verify = {100LL, 3600LL, 99991LL};
    for (long long n : verify) {
        auto factors = prime_factorization(n, rng);
        long long product = 1;
        for (auto& [p, e] : factors) {
            for (int i = 0; i < e; ++i) product *= p;
        }
        std::cout << "  " << n << " -> " << (product == n ? "OK" : "FAIL") << "\n";
    }

    return 0;
}
