#pragma once

#include <print>
#include <vector>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include <numeric>

namespace randalgo {

// ---------------------------------------------------------------------------
// 1. Modular Exponentiation -- binary method, O(log exp)
//    Computes (base^exp) mod mod_val using repeated squaring.
// ---------------------------------------------------------------------------
long long mod_pow(long long base, long long exp, long long mod_val) {
    if (mod_val == 1) return 0;
    long long result = 1;
    base %= mod_val;
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % mod_val;
        }
        base = (base * base) % mod_val;
        exp >>= 1;
    }
    return result;
}

// ---------------------------------------------------------------------------
// 2. GCD -- Euclidean algorithm
// ---------------------------------------------------------------------------
long long gcd(long long a, long long b) {
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        a %= b;
        std::swap(a, b);
    }
    return a;
}

// ---------------------------------------------------------------------------
// 3. Extended GCD -- finds (g, x, y) such that ax + by = g = gcd(a, b)
// ---------------------------------------------------------------------------
struct ExtendedGCDResult {
    long long g; // gcd
    long long x; // coefficient for a
    long long y; // coefficient for b
};

ExtendedGCDResult extended_gcd(long long a, long long b) {
    if (a == 0) return {b, 0, 1};
    auto [g, x1, y1] = extended_gcd(b % a, a);
    long long x = y1 - (b / a) * x1;
    long long y = x1;
    return {g, x, y};
}

// ---------------------------------------------------------------------------
// 4. Modular Inverse -- using extended Euclidean algorithm
//    Returns x such that (a * x) == 1 (mod mod_val).
// ---------------------------------------------------------------------------
long long mod_inverse(long long a, long long mod_val) {
    auto [g, x, y] = extended_gcd(a, mod_val);
    if (g != 1) {
        throw std::runtime_error("Modular inverse does not exist (gcd != 1)");
    }
    return ((x % mod_val) + mod_val) % mod_val;
}

// ---------------------------------------------------------------------------
// 5. Trial Division Primality Test
//    Tests divisibility by 2, 3, then all numbers of the form 6k +/- 1.
// ---------------------------------------------------------------------------
bool is_prime_trial(long long n) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// 6. Chinese Remainder Theorem
//    Solves the system: x == a_i (mod n_i) for pairwise coprime n_i.
//    Uses constructive method: M = prod(n_i), x = sum(a_i * M_i * y_i)
//    where M_i = M/n_i and y_i = M_i^{-1} mod n_i.
// ---------------------------------------------------------------------------
struct Congruence {
    long long a; // remainder
    long long n; // modulus
};

long long chinese_remainder_theorem(const std::vector<Congruence>& system) {
    if (system.empty()) {
        throw std::runtime_error("Empty system of congruences");
    }

    long long M = 1;
    for (const auto& c : system) {
        M *= c.n;
    }

    long long x = 0;
    for (const auto& c : system) {
        long long M_i = M / c.n;
        long long y_i = mod_inverse(M_i, c.n);
        x = (x + c.a * M_i % M * y_i % M) % M;
    }
    return ((x % M) + M) % M;
}

// ---------------------------------------------------------------------------
// 7. Euler's Totient Function -- compute phi(n)
//    phi(n) = n * prod((1 - 1/p)) for all prime factors p of n.
// ---------------------------------------------------------------------------
long long euler_totient(long long n) {
    long long result = n;
    for (long long p = 2; p * p <= n; ++p) {
        if (n % p == 0) {
            while (n % p == 0) {
                n /= p;
            }
            result -= result / p;
        }
    }
    if (n > 1) {
        result -= result / n;
    }
    return result;
}

// ---------------------------------------------------------------------------
// 8. Demonstration -- tests all number theory routines
// ---------------------------------------------------------------------------
void demonstrate_number_theory() {
    std::println("=== Number Theory Algorithms ===\n");

    // --- Modular Exponentiation ---
    std::println("--- Modular Exponentiation ---");
    {
        long long base = 2, exp = 10, mod = 1000;
        std::println("  2^10 mod 1000 = {}", mod_pow(base, exp, mod));

        base = 3; exp = 13; mod = 7;
        std::println("  3^13 mod 7    = {}", mod_pow(base, exp, mod));

        base = 7; exp = 256; mod = 13;
        std::println("  7^256 mod 13  = {}", mod_pow(base, exp, mod));
    }

    // --- GCD ---
    std::println("\n--- GCD (Euclidean Algorithm) ---");
    {
        std::println("  gcd(48, 18)    = {}", gcd(48, 18));
        std::println("  gcd(100, 75)   = {}", gcd(100, 75));
        std::println("  gcd(17, 13)    = {}", gcd(17, 13));
    }

    // --- Extended GCD ---
    std::println("\n--- Extended GCD ---");
    {
        auto [g, x, y] = extended_gcd(35, 15);
        std::println("  35 * ({}) + 15 * ({}) = {} = gcd(35, 15)", x, y, g);

        auto [g2, x2, y2] = extended_gcd(30, 20);
        std::println("  30 * ({}) + 20 * ({}) = {} = gcd(30, 20)", x2, y2, g2);

        auto [g3, x3, y3] = extended_gcd(99, 78);
        std::println("  99 * ({}) + 78 * ({}) = {} = gcd(99, 78)", x3, y3, g3);
    }

    // --- Modular Inverse ---
    std::println("\n--- Modular Inverse ---");
    {
        long long a = 3, mod = 7;
        long long inv = mod_inverse(a, mod);
        std::println("  {}^(-1) mod {} = {}  (check: {} * {} mod {} = {})",
                      a, mod, inv, a, inv, mod, (a * inv) % mod);

        a = 17; mod = 43;
        inv = mod_inverse(a, mod);
        std::println("  {}^(-1) mod {} = {}  (check: {} * {} mod {} = {})",
                      a, mod, inv, a, inv, mod, (a * inv) % mod);
    }

    // --- Primality ---
    std::println("\n--- Trial Division Primality ---");
    {
        std::vector<long long> test_nums = {1, 2, 3, 4, 17, 25, 29, 100, 997, 1000};
        for (long long n : test_nums) {
            std::println("  is_prime({}) = {}", n, is_prime_trial(n) ? "true" : "false");
        }
    }

    // --- Chinese Remainder Theorem ---
    std::println("\n--- Chinese Remainder Theorem ---");
    {
        // Solve: x == 2 (mod 3), x == 3 (mod 5), x == 2 (mod 7)
        std::vector<Congruence> system = {{2, 3}, {3, 5}, {2, 7}};
        long long x = chinese_remainder_theorem(system);
        std::println("  x == 2 (mod 3), x == 3 (mod 5), x == 2 (mod 7)");
        std::println("  Solution: x = {} (mod {})", x, 3 * 5 * 7);

        // Verify
        std::println("  Verify: {} mod 3 = {}, {} mod 5 = {}, {} mod 7 = {}",
                      x, x % 3, x, x % 5, x, x % 7);
    }

    // --- Euler Totient ---
    std::println("\n--- Euler's Totient Function ---");
    {
        std::vector<long long> nums = {1, 2, 6, 10, 12, 36, 100};
        for (long long n : nums) {
            std::println("  phi({}) = {}", n, euler_totient(n));
        }
    }
}

} // namespace randalgo
