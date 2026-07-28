// example_rsa_encryption.cpp
//
// Full RSA Encryption/Decryption Demo
// ======================================
// RSA is a public-key cryptosystem based on the difficulty of factoring
// large numbers. Key steps:
//
//   1. Key Generation:
//      - Choose two distinct primes p, q
//      - Compute n = p * q  (modulus)
//      - Compute φ(n) = (p-1)(q-1)
//      - Choose public exponent e with gcd(e, φ(n)) = 1
//      - Compute private exponent d = e^(-1) mod φ(n)
//
//   2. Encryption:  ciphertext = message^e mod n
//   3. Decryption:  message = ciphertext^d mod n
//
// Correctness: m^(e*d) ≡ m (mod n) because e*d ≡ 1 (mod φ(n)),
// which follows from Euler's theorem.

#include "ral/crypto.h"
#include <iostream>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Full RSA Encryption/Decryption Demo\n";
    std::cout << "========================================================\n\n";

    // --- Key Generation ---
    long long p = 61, q = 53;
    long long pub_e = 17;
    auto [e, d, n] = generate_rsa_keypair(p, q, pub_e);

    std::cout << "--- Key Generation ---\n";
    std::cout << "  Primes:    p = " << p << ", q = " << q << "\n";
    std::cout << "  Modulus:   n = p * q = " << n << "\n";
    std::cout << "  φ(n):      " << (p - 1) * (q - 1) << "\n";
    std::cout << "  Public:    (e=" << e << ", n=" << n << ")\n";
    std::cout << "  Private:   (d=" << d << ", n=" << n << ")\n";

    // --- Encrypt and Decrypt messages ---
    std::cout << "\n--- Encryption/Decryption ---\n";
    long long messages[] = {42, 65, 100, 255, 1234};
    for (long long msg : messages) {
        long long cipher = rsa_encrypt(msg, e, n);
        long long plain  = rsa_decrypt(cipher, d, n);
        std::cout << "  Message:     " << msg << "\n";
        std::cout << "  Ciphertext:  " << cipher << "\n";
        std::cout << "  Decrypted:   " << plain << "\n";
        std::cout << "  Match:       " << (plain == msg ? "YES" : "NO") << "\n\n";
    }

    // --- Verify mathematical correctness ---
    std::cout << "--- Mathematical Verification ---\n";
    std::cout << "  e * d mod φ(n) = " << e << " * " << d
              << " mod " << (p - 1) * (q - 1) << " = "
              << (e * d) % ((p - 1) * (q - 1)) << "\n";
    std::cout << "  (should be 1, confirming e*d ≡ 1 mod φ(n))\n";

    return 0;
}
