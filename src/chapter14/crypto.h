#pragma once

#include "number_theory.h"

#include <print>
#include <vector>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <chrono>

namespace randalgo {

// ---------------------------------------------------------------------------
// Shared PRNG -- seeded from high-resolution clock for reproducibility demos
// ---------------------------------------------------------------------------
static inline std::mt19937_64& crypto_rng() {
    static std::mt19937_64 rng{
        static_cast<unsigned>(
            std::chrono::steady_clock::now().time_since_epoch().count())};
    return rng;
}

// ---------------------------------------------------------------------------
// 1. Quadratic Residue -- Euler's criterion: a is a QR mod p iff
//    a^((p-1)/2) == 1 (mod p).
// ---------------------------------------------------------------------------
bool is_quadratic_residue(long long a, long long p) {
    if (a < 0 || a >= p) a = ((a % p) + p) % p;
    if (a == 0) return true;
    long long euler = mod_pow(a, (p - 1) / 2, p);
    return euler == 1;
}

// ---------------------------------------------------------------------------
// 2. Legendre Symbol (a / p)
//    Returns  1  if a is a QR mod p
//            -1  if a is a QNR mod p
//             0  if p | a
// ---------------------------------------------------------------------------
int legendre_symbol(long long a, long long p) {
    if (!is_prime_trial(p)) {
        throw std::runtime_error("Legendre symbol requires prime modulus");
    }
    if (a < 0 || a >= p) a = ((a % p) + p) % p;
    if (a == 0) return 0;
    long long euler = mod_pow(a, (p - 1) / 2, p);
    if (euler == p - 1) return -1;
    return static_cast<int>(euler);
}

// ---------------------------------------------------------------------------
// 3. Jacobi Symbol (a / n) -- generalization of Legendre to composite n
//    Uses properties:
//      (a / n) = (a mod n / n)
//      (0 / n) = 0
//      (1 / n) = 1
//      (2a / n) = (-1)^((n^2-1)/8) * (a / n)  [if n is odd]
//      (a / n) = (b / a) * (-1)^((a-1)(n-1)/4)  [quadratic reciprocity]
// ---------------------------------------------------------------------------
int jacobi_symbol(long long a, long long n) {
    if (n <= 0 || n % 2 == 0) {
        throw std::runtime_error("Jacobi symbol requires odd positive n");
    }
    a = ((a % n) + n) % n;
    int result = 1;
    while (a != 0) {
        // Factor out powers of 2 from a
        int v = 0;
        while (a % 2 == 0) {
            a /= 2;
            ++v;
        }
        // Apply (2 / n): (-1)^((n^2-1)/8)
        if (v % 2 == 1) {
            long long n_mod8 = n % 8;
            if (n_mod8 == 3 || n_mod8 == 5) {
                result = -result;
            }
        }
        // Quadratic reciprocity swap
        if (a % 4 == 3 && n % 4 == 3) {
            result = -result;
        }
        std::swap(a, n);
        a %= n;
    }
    return (n == 1) ? result : 0;
}

// ---------------------------------------------------------------------------
// 4. RSA Key Generation
//    Given primes p, q and public exponent e (with gcd(e, phi(n)) = 1),
//    compute the private exponent d = e^(-1) mod phi(n).
// ---------------------------------------------------------------------------
struct RSAKeyPair {
    long long e;  // public exponent
    long long d;  // private exponent
    long long n;  // modulus
};

RSAKeyPair generate_rsa_keypair(long long p, long long q, long long e) {
    long long n = p * q;
    long long phi_n = (p - 1) * (q - 1);

    if (gcd(e, phi_n) != 1) {
        throw std::runtime_error("e must be coprime to phi(n)");
    }

    long long d = mod_inverse(e, phi_n);
    return {e, d, n};
}

// ---------------------------------------------------------------------------
// 5. RSA Encrypt / Decrypt
//    Encrypt: c = m^e mod n
//    Decrypt: m = c^d mod n
// ---------------------------------------------------------------------------
long long rsa_encrypt(long long m, long long e, long long n) {
    return mod_pow(m, e, n);
}

long long rsa_decrypt(long long c, long long d, long long n) {
    return mod_pow(c, d, n);
}

// ---------------------------------------------------------------------------
// 6. Miller-Rabin Primality Test
//    Tests whether n is prime using k random bases.
//    Writes n-1 = 2^r * d, then checks:
//      a^d != 1 (mod n) AND a^(2^j * d) != -1 (mod n)  for all 0 <= j < r
//    then n is composite.
// ---------------------------------------------------------------------------
bool miller_rabin(long long n, int k = 20) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;

    // Write n - 1 = 2^r * d
    long long d = n - 1;
    int r = 0;
    while (d % 2 == 0) {
        d /= 2;
        ++r;
    }

    std::uniform_int_distribution<long long> dist(2, n - 2);

    for (int i = 0; i < k; ++i) {
        long long a = dist(crypto_rng());
        long long x = mod_pow(a, d, n);

        if (x == 1 || x == n - 1) continue;

        bool found = false;
        for (int j = 0; j < r - 1; ++j) {
            x = mod_pow(x, 2, n);
            if (x == n - 1) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// 7. Solovay-Strassen Primality Test
//    n is composite if for some random a:
//      a^((n-1)/2) != (a / n) (mod n)
// ---------------------------------------------------------------------------
bool solovay_strassen(long long n, int k = 20) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;

    std::uniform_int_distribution<long long> dist(2, n - 2);

    for (int i = 0; i < k; ++i) {
        long long a = dist(crypto_rng());
        long long a_mod_n = ((a % n) + n) % n;
        long long euler = mod_pow(a_mod_n, (n - 1) / 2, n);
        int js = jacobi_symbol(a, n);
        // Treat -1 as n-1 in modular arithmetic for comparison
        long long js_mod_n = (js + n) % n;
        if (euler != js_mod_n) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// 8. Fermat Primality Test
//    n is composite if a^(n-1) != 1 (mod n) for some random a.
// ---------------------------------------------------------------------------
bool fermat_primality_test(long long n, int k = 20) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;

    std::uniform_int_distribution<long long> dist(2, n - 2);

    for (int i = 0; i < k; ++i) {
        long long a = dist(crypto_rng());
        if (mod_pow(a, n - 1, n) != 1) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// 9. Demonstration
// ---------------------------------------------------------------------------
void demonstrate_crypto() {
    std::println("=== Cryptographic Algorithms ===\n");

    // --- RSA Key Generation & Encryption/Decryption ---
    std::println("--- RSA Encryption/Decryption ---");
    {
        long long p = 61, q = 53;
        long long e = 17;
        auto [pub_e, priv_d, n] = generate_rsa_keypair(p, q, e);
        std::println("  p = {}, q = {}, n = {}, phi(n) = {}", p, q, n, (p - 1) * (q - 1));
        std::println("  Public key:  (e={}, n={})", pub_e, n);
        std::println("  Private key: (d={}, n={})", priv_d, n);

        long long msg = 42;
        long long cipher = rsa_encrypt(msg, pub_e, n);
        long long plain  = rsa_decrypt(cipher, priv_d, n);
        std::println("  Message:     {}", msg);
        std::println("  Ciphertext:  {}", cipher);
        std::println("  Decrypted:   {}", plain);
        std::println("  Roundtrip OK: {}", plain == msg ? "YES" : "NO");

        // Second message
        msg = 255;
        cipher = rsa_encrypt(msg, pub_e, n);
        plain  = rsa_decrypt(cipher, priv_d, n);
        std::println("\n  Message:     {}", msg);
        std::println("  Ciphertext:  {}", cipher);
        std::println("  Decrypted:   {}", plain);
    }

    // --- Primality Tests ---
    std::println("\n--- Primality Tests ---");
    {
        std::vector<long long> candidates = {2, 3, 17, 561, 997, 1009, 104729};
        std::println("  {:>8} | {:>12} | {:>12} | {:>12} | {:>8}",
                      "n", "Fermat", "Miller-Rabin", "Solovay-Str", "Trial");
        std::println("  {}-+-{}-+-{}-+-{}-+-{}",
                      std::string(8, '-'), std::string(12, '-'),
                      std::string(12, '-'), std::string(12, '-'), std::string(8, '-'));

        for (long long n : candidates) {
            bool f = fermat_primality_test(n, 20);
            bool mr = miller_rabin(n, 20);
            bool ss = solovay_strassen(n, 20);
            bool td = is_prime_trial(n);
            std::println("  {:>8} | {:>12} | {:>12} | {:>12} | {:>8}",
                          n,
                          f  ? "prime" : "composite",
                          mr ? "prime" : "composite",
                          ss ? "prime" : "composite",
                          td ? "prime" : "composite");
        }

        // Carmichael number 561 -- Fermat test fails with some bases
        std::println("\n  Note: 561 is a Carmichael number (composite but passes Fermat for many bases)");
    }

    // --- Quadratic Residues ---
    std::println("\n--- Quadratic Residues ---");
    {
        long long p = 13;
        std::println("  Modulus p = {}", p);
        std::println("  Quadratic residues mod {}:", p);

        for (long long a = 0; a < p; ++a) {
            bool qr = is_quadratic_residue(a, p);
            int ls = legendre_symbol(a, p);
            std::println("    a={}: QR={}  Legendre={}", a, qr ? "yes" : "no ", ls);
        }

        // Legendre symbol for several values
        std::println("\n  Legendre symbols (a / 7):");
        for (long long a = 1; a <= 6; ++a) {
            std::println("    ({} / 7) = {}", a, legendre_symbol(a, 7));
        }

        // Jacobi symbol for composite modulus
        std::println("\n  Jacobi symbols (a / 15) with n=15=3x5:");
        for (long long a = 1; a <= 14; ++a) {
            int js = jacobi_symbol(a, 15);
            std::println("    ({} / 15) = {}", a, js);
        }
    }
}

} // namespace randalgo
