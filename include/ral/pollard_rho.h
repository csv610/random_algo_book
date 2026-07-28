#pragma once

#include <numeric>
#include <cmath>
#include <iostream>
#include <vector>
#include "number_theory.h"
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Pollard's Rho Algorithm for Integer Factorization
// Finds a non-trivial factor of n in expected O(n^{1/4}) time.
// Uses Floyd's cycle detection (tortoise and hare).
// ---------------------------------------------------------------------------

// Modular multiplication using __int128 to avoid overflow
inline long long mulmod(long long a, long long b, long long mod) {
    return static_cast<long long>((__int128)a * b % mod);
}

// Pollard's rho: find a factor of n (assumes n is composite, n > 1)
// Returns a non-trivial factor, or n if it fails (should retry with different c)
inline long long pollard_rho_factor(long long n, std::mt19937& rng) {
    if (n % 2 == 0) return 2;

    std::uniform_int_distribution<long long> cdist(1, n - 1);
    long long c = cdist(rng);

    auto f = [&](long long x) -> long long {
        return (mulmod(x, x, n) + c) % n;
    };

    long long x = cdist(rng);
    long long y = x;
    long long d = 1;

    while (d == 1) {
        x = f(x);       // tortoise: one step
        y = f(f(y));     // hare: two steps
        d = std::gcd(std::abs(x - y), n);
    }

    if (d == n) {
        // Failed with this c, retry with a different one
        return pollard_rho_factor(n, rng);
    }
    return d;
}

// Factor n completely, returns sorted list of prime factors (with multiplicity)
inline std::vector<long long> pollard_rho_factorize(long long n, std::mt19937& rng) {
    if (n <= 1) return {};

    // First do trial division for small primes
    std::vector<long long> factors;
    for (long long p : {2LL, 3LL, 5LL, 7LL, 11LL, 13LL, 17LL, 19LL, 23LL, 29LL, 31LL, 37LL}) {
        while (n % p == 0) {
            factors.push_back(p);
            n /= p;
        }
    }

    // Use trial division up to some bound
    for (long long d = 41; d * d <= n && d < 10000; d += 2) {
        while (n % d == 0) {
            factors.push_back(d);
            n /= d;
        }
    }

    if (n <= 1) {
        std::sort(factors.begin(), factors.end());
        return factors;
    }

    if (is_prime_trial(n)) {
        factors.push_back(n);
        std::sort(factors.begin(), factors.end());
        return factors;
    }

    // Factor the remaining composite using Pollard's rho
    std::vector<long long> stack = {n};
    while (!stack.empty()) {
        long long cur = stack.back();
        stack.pop_back();

        if (cur <= 1) continue;
        if (is_prime_trial(cur)) {
            factors.push_back(cur);
            continue;
        }

        long long d = pollard_rho_factor(cur, rng);
        stack.push_back(d);
        stack.push_back(cur / d);
    }

    std::sort(factors.begin(), factors.end());
    return factors;
}

// Get unique prime factors with their exponents
struct PrimePower {
    long long prime;
    int exponent;
};

inline std::vector<PrimePower> prime_factorization(long long n, std::mt19937& rng) {
    auto factors = pollard_rho_factorize(n, rng);
    std::vector<PrimePower> result;

    size_t i = 0;
    while (i < factors.size()) {
        long long p = factors[i];
        int exp = 0;
        while (i < factors.size() && factors[i] == p) {
            exp++;
            i++;
        }
        result.push_back({p, exp});
    }
    return result;
}

// Demonstration
inline void demonstrate_pollard_rho() {
    std::mt19937 rng(42);

    println("=== Pollard's Rho Factorization ===\n");

    std::vector<long long> test_numbers = {
        91,           // 7 * 13
        100,          // 2^2 * 5^2
        997,          // prime
        1000003LL,    // prime
        1000000007LL, // prime (10^9 + 7)
        2147483647LL, // Mersenne prime (2^31 - 1)
        123456789LL,  // 3^2 * 3607 * 3803
        982451653LL,  // prime
        999999937LL,  // prime
        600851475143LL // 71 * 839 * 1471 * 6857
    };

    for (long long n : test_numbers) {
        auto t_start = std::chrono::high_resolution_clock::now();
        auto fp = prime_factorization(n, rng);
        auto t_end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(t_end - t_start).count();

        print("  {} = ", n);
        bool first = true;
        for (auto& [p, e] : fp) {
            if (!first) print(" * ");
            first = false;
            if (e == 1) {
                print("{}", p);
            } else {
                print("{}^{}", p, e);
            }
        }
        println("  ({:.2f} ms)", elapsed);
    }

    // Verify factorizations
    println("\n--- Verification ---");
    for (long long n : test_numbers) {
        auto fp = prime_factorization(n, rng);
        long long product = 1;
        for (auto& [p, e] : fp) {
            for (int i = 0; i < e; ++i) product *= p;
        }
        println("  {} factored correctly: {}", n, product == n ? "YES" : "NO");
    }
}

} // namespace ral
