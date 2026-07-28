// example_pollard_rho_factorize.cpp
// Full Factorization using Pollard's Rho + Trial Division
//
// Combines trial division for small primes with Pollard's rho for
// large composite factors. Returns a sorted list of all prime factors
// with multiplicity. Uses a stack-based approach to recursively factor
// composite remainders.
//
// Compile: g++ -std=c++23 -I../include -o example_pollard_rho_factorize example_pollard_rho_factorize.cpp

#include <random>
#include <ral/pollard_rho.h>
#include <iostream>
#include <numeric>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Full Factorization (Pollard's Rho + Trial Division) ===\n\n";

    std::vector<long long> numbers = {
        2LL,
        100LL,
        997LL,
        12345LL,
        1000003LL,
        123456789LL,
        982451653LL,
        600851475143LL,
    };

    for (long long n : numbers) {
        auto factors = pollard_rho_factorize(n, rng);

        std::cout << "  " << n << " = ";
        bool first = true;
        for (int f : factors) {
            if (!first) std::cout << " * ";
            first = false;
            std::cout << f;
        }

        // Verify
        long long product = 1;
        for (long long f : factors) product *= f;
        std::cout << "  [verify: " << (product == n ? "OK" : "FAIL") << "]\n";
    }

    // Show prime_factorization (with exponents)
    std::cout << "\nWith exponents:\n";
    std::vector<long long> more_nums = {100LL, 360LL, 1024LL, 13195LL, 600851475143LL};
    for (long long n : more_nums) {
        auto pp = prime_factorization(n, rng);
        std::cout << "  " << n << " = ";
        bool first = true;
        for (auto& [p, e] : pp) {
            if (!first) std::cout << " * ";
            first = false;
            std::cout << p;
            if (e > 1) std::cout << "^" << e;
        }
        std::cout << "\n";
    }

    return 0;
}
