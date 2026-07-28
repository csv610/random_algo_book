// example_rsa_encrypt_decrypt.cpp
//
// RSA Encrypt/Decrypt Round-Trip Demonstration
// ===============================================
// Full demonstration of RSA encryption and decryption:
//   1. Generate a key pair from two primes
//   2. Encrypt several messages
//   3. Decrypt and verify correctness
//   4. Show ciphertext ≠ plaintext (encryption actually changes data)
//   5. Demonstrate that wrong key fails to decrypt
//
// RSA formulas:
//   Encrypt: c = m^e mod n
//   Decrypt: m = c^d mod n
//   Correctness: c^d = m^(ed) = m^(1 + kφ(n)) = m * (m^φ(n))^k ≡ m * 1^k = m (mod n)

#include "ral/crypto.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  RSA Encrypt/Decrypt Round-Trip Demo\n";
    std::cout << "========================================================\n\n";

    // --- Setup ---
    long long p = 61, q = 53;
    long long e = 17;
    auto [pub_e, priv_d, n] = generate_rsa_keypair(p, q, e);

    std::cout << "--- Key Setup ---\n";
    std::cout << "  p = " << p << ", q = " << q << ", n = " << n << "\n";
    std::cout << "  Public key:  (e=" << pub_e << ", n=" << n << ")\n";
    std::cout << "  Private key: (d=" << priv_d << ", n=" << n << ")\n\n";

    // --- Encrypt/Decrypt multiple messages ---
    std::cout << "--- Round-Trip Encryption/Decryption ---\n";
    std::vector<long long> messages = {0, 1, 2, 42, 65, 100, 255, 3232};

    std::cout << "  msg  | ciphertext | decrypted | match\n";
    std::cout << "  -----+------------+-----------+------\n";

    for (long long msg : messages) {
        long long cipher = rsa_encrypt(msg, pub_e, n);
        long long plain  = rsa_decrypt(cipher, priv_d, n);
        std::cout << "  " << msg << "   |    " << cipher
                  << "       |     " << plain
                  << "     | " << (plain == msg ? "yes" : "NO") << "\n";
    }

    // --- Encryption properties ---
    std::cout << "\n--- Encryption Properties ---\n";
    {
        long long m1 = 10, m2 = 20;
        long long c1 = rsa_encrypt(m1, pub_e, n);
        long long c2 = rsa_encrypt(m2, pub_e, n);
        long long c12 = rsa_encrypt(m1 + m2, pub_e, n);
        std::cout << "  RSA is NOT homomorphic under addition:\n";
        std::cout << "    enc(" << m1 << ") + enc(" << m2 << ") = " << c1 << " + " << c2
                  << " = " << (c1 + c2) << "\n";
        std::cout << "    enc(" << m1 << " + " << m2 << ") = enc(" << (m1 + m2) << ") = " << c12 << "\n";
        std::cout << "    " << (c1 + c2 == c12 ? "Homomorphic (unexpected!)" : "Not equal (expected)") << "\n";
    }

    // --- Wrong key decryption ---
    std::cout << "\n--- Wrong Key Decryption Attempt ---\n";
    {
        long long msg = 42;
        long long cipher = rsa_encrypt(msg, pub_e, n);

        // Try decrypting with a different (wrong) d
        long long wrong_d = priv_d + 1;
        long long wrong_plain = rsa_decrypt(cipher, wrong_d, n);
        std::cout << "  Message:     " << msg << "\n";
        std::cout << "  Ciphertext:  " << cipher << "\n";
        std::cout << "  Wrong d:     " << wrong_d << "\n";
        std::cout << "  'Decrypted': " << wrong_plain
                  << (wrong_plain == msg ? "  (matches by luck)" : "  (garbage — correct!)") << "\n";
    }

    // --- Message must be < n ---
    std::cout << "\n--- Message Size Constraint ---\n";
    std::cout << "  n = " << n << ", so messages must be in [0, " << (n - 1) << "]\n";
    std::cout << "  Attempting to encrypt message = " << n << ":\n";
    {
        long long msg = n;
        long long cipher = rsa_encrypt(msg, pub_e, n);
        long long plain = rsa_decrypt(cipher, priv_d, n);
        std::cout << "    enc(" << msg << ") = " << cipher << "\n";
        std::cout << "    dec(" << cipher << ") = " << plain << "\n";
        std::cout << "    " << (plain == msg ? "Matched" : "Different — msg was reduced mod n") << "\n";
    }

    return 0;
}
