// example_rsa_keygen.cpp
//
// RSA Key Generation with Different Prime Sizes
// ================================================
// Demonstrates RSA key generation with primes of increasing size,
// showing how key size affects the modulus and key space.
//
// Key generation steps:
//   1. Choose two distinct primes p, q
//   2. n = p * q (the modulus, its bit-length is the "key size")
//   3. φ(n) = (p-1)(q-1)
//   4. Choose e (commonly 65537 for efficiency)
//   5. d = e^(-1) mod φ(n) (private exponent)

#include "ral/crypto.h"
#include <iostream>
#include <vector>
#include <string>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  RSA Key Generation Demo\n";
    std::cout << "========================================================\n\n";

    struct KeySpec {
        std::string label;
        long long p, q, e;
    };

    std::vector<KeySpec> specs = {
        {"Tiny (educational)", 61, 53, 17},
        {"Small", 101, 103, 17},
        {"Medium", 1049, 1061, 65537},
        {"Larger", 4999, 5003, 65537},
    };

    for (auto& [label, p, q, e] : specs) {
        std::cout << "--- " << label << " ---\n";
        auto [pub_e, priv_d, n] = generate_rsa_keypair(p, q, e);
        long long phi_n = (p - 1) * (q - 1);

        std::cout << "  p       = " << p << "\n";
        std::cout << "  q       = " << q << "\n";
        std::cout << "  n = pq  = " << n << "\n";
        std::cout << "  φ(n)    = " << phi_n << "\n";
        std::cout << "  e       = " << pub_e << "\n";
        std::cout << "  d       = " << priv_d << "\n";
        std::cout << "  Key bits: ~" << 0 << " (n has " << n << ")\n";

        // Verify roundtrip
        long long msg = 42;
        long long cipher = rsa_encrypt(msg, pub_e, n);
        long long plain = rsa_decrypt(cipher, priv_d, n);
        std::cout << "  Roundtrip(42): " << msg << " → " << cipher << " → " << plain
                  << (plain == msg ? "  OK" : "  FAIL") << "\n\n";
    }

    // --- Why e = 65537? ---
    std::cout << "--- Why e = 65537? ---\n";
    std::cout << "  65537 = 2^16 + 1 (prime, only 17 bits set in binary: 10000000000000001)\n";
    std::cout << "  This makes encryption (m^e mod n) efficient via square-and-multiply:\n";
    std::cout << "  Only 17 squarings + 1 multiplication needed.\n";
    std::cout << "  It's also large enough to avoid Wiener's attack on small d.\n\n";

    // --- Small e attack warning ---
    std::cout << "--- Why NOT to Use Small e (e=3) with Small Messages ---\n";
    {
        long long p = 61, q = 53;
        auto [pub_e, priv_d, n] = generate_rsa_keypair(p, q, 3);
        std::cout << "  e=3, n=" << n << "\n";

        // If m^3 < n, then m^3 mod n = m^3 (no reduction), so m = cbrt(c)
        long long msg = 10;
        long long cipher = rsa_encrypt(msg, 3, n);
        std::cout << "  Message " << msg << ": " << msg << "^3 = "
                  << msg * msg * msg;
        if (msg * msg * msg < n) {
            std::cout << " < " << n << " (no modular reduction!)\n";
            std::cout << "  Attacker can just compute cube root of ciphertext!\n";
        } else {
            std::cout << " >= " << n << " (modular reduction occurred)\n";
        }
    }

    // --- Larger primes ---
    std::cout << "\n--- Larger Primes ---\n";
    {
        long long p = 104729, q = 104743;
        long long e = 65537;
        long long n = p * q;
        long long phi_n = (p - 1) * (q - 1);
        std::cout << "  p = " << p << ", q = " << q << "\n";
        std::cout << "  n = " << n << "\n";
        std::cout << "  φ(n) = " << phi_n << "\n";
        std::cout << "  e = " << e << "\n";
        long long d = mod_inverse(e, phi_n);
        std::cout << "  d = " << d << "\n";
        std::cout << "  e * d mod φ(n) = " << (e * d) % phi_n << "\n";
    }

    return 0;
}
