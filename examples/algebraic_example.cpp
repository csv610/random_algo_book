#include "ral.h"
#include <iostream>
#include <vector>
#include <string>
#include <random>

int main() {
    std::cout << "========================================================\n";
    std::cout << "        RAL Algebraic & Crypto Examples\n";
    std::cout << "========================================================\n\n";

    std::mt19937 rng(42);

    // ----------------------------------------------------------------
    // 1. Freivalds' Matrix Verification
    // ----------------------------------------------------------------
    std::cout << "--- 1. Freivalds' Matrix Verification ---\n";
    std::vector<std::vector<long long>> A = {{1, 2}, {3, 4}};
    std::vector<std::vector<long long>> B = {{5, 6}, {7, 8}};
    std::vector<std::vector<long long>> C = {{19, 22}, {43, 50}}; // Correct product AB
    std::vector<std::vector<long long>> C_wrong = {{19, 22}, {43, 51}}; // Incorrect

    bool correct_pass = ral::freivalds_verify(A, B, C, 10, rng);
    bool wrong_pass = ral::freivalds_verify(A, B, C_wrong, 10, rng);

    std::cout << "  Verifying correct AB = C: " << (correct_pass ? "Passed (Correct)" : "Failed") << "\n";
    std::cout << "  Verifying incorrect AB = C_wrong: " << (wrong_pass ? "Passed" : "Failed (Detected Error)") << "\n\n";

    // ----------------------------------------------------------------
    // 2. Schwartz-Zippel Polynomial Identity Testing (PIT)
    // ----------------------------------------------------------------
    std::cout << "--- 2. Schwartz-Zippel Polynomial Identity Test ---\n";
    // Check if (x + 1)^2 == x^2 + 2x + 1
    // Represented as coefficient vectors (constant coefficient first):
    // p1 = (x + 1)^2 = 1 + 2x + x^2 => {1, 2, 1}
    // p2 = x^2 + 2x + 1 => {1, 2, 1}
    std::vector<int> p1 = {1, 2, 1};
    std::vector<int> p2 = {1, 2, 1};
    std::vector<int> p3 = {1, 3, 1}; // x^2 + 3x + 1 (different)

    bool ident_12 = ral::polynomial_identity_test(p1, p2, 1000, 5, rng);
    bool ident_13 = ral::polynomial_identity_test(p1, p3, 1000, 5, rng);

    std::cout << "  Testing if p1 == p2: " << (ident_12 ? "Identical" : "Different") << "\n";
    std::cout << "  Testing if p1 == p3: " << (ident_13 ? "Identical" : "Different") << "\n\n";

    // ----------------------------------------------------------------
    // 3. Rabin-Karp Pattern Matching
    // ----------------------------------------------------------------
    std::cout << "--- 3. Rabin-Karp String Matching ---\n";
    std::string text = "ABABDABACDABABCABAB";
    std::string pattern = "ABABCABAB";
    std::vector<int> matches = ral::rabin_karp_search(text, pattern);
    std::cout << "  Text: " << text << "\n";
    std::cout << "  Pattern: " << pattern << "\n";
    std::cout << "  Matches found at index: ";
    for (int idx : matches) {
        std::cout << idx << " ";
    }
    std::cout << "\n\n";

    // ----------------------------------------------------------------
    // 4. Modular Polynomial Arithmetic
    // ----------------------------------------------------------------
    std::cout << "--- 4. Modular Polynomial Arithmetic (mod 17) ---\n";
    long long prime = 17;
    // polyA = 3x^2 + 2x + 1 => {1, 2, 3}
    // polyB = 5x + 4 => {4, 5}
    ral::Polynomial polyA({1, 2, 3});
    ral::Polynomial polyB({4, 5});

    auto polySum = polyA.add(polyB, prime);
    auto polyProd = polyA.multiply(polyB, prime);

    std::cout << "  A(x) = " << polyA.to_string() << "\n";
    std::cout << "  B(x) = " << polyB.to_string() << "\n";
    std::cout << "  A(x) + B(x) mod 17 = " << polySum.to_string() << "\n";
    std::cout << "  A(x) * B(x) mod 17 = " << polyProd.to_string() << "\n";
    std::cout << "  A(10) mod 17 = " << polyA.evaluate(10, prime) << "\n\n";

    // ----------------------------------------------------------------
    // 5. Basic Number Theory & Cryptography
    // ----------------------------------------------------------------
    std::cout << "--- 5. Number Theory & RSA Crypto ---\n";
    // CRT: x == 2 (mod 3), x == 3 (mod 5), x == 2 (mod 7)
    std::vector<ral::Congruence> crt_system = {{2, 3}, {3, 5}, {2, 7}};
    long long crt_sol = ral::chinese_remainder_theorem(crt_system);
    std::cout << "  CRT solution: x = " << crt_sol << " (mod 105)\n";

    // RSA: P=61, Q=53, E=17
    long long rsa_p = 61, rsa_q = 53, rsa_e = 17;
    auto rsa_key = ral::generate_rsa_keypair(rsa_p, rsa_q, rsa_e);
    std::cout << "  RSA Public Key: (e=" << rsa_key.e << ", n=" << rsa_key.n << ")\n";
    std::cout << "  RSA Private Key: (d=" << rsa_key.d << ", n=" << rsa_key.n << ")\n";

    long long message = 42;
    long long encrypted = ral::rsa_encrypt(message, rsa_key.e, rsa_key.n);
    long long decrypted = ral::rsa_decrypt(encrypted, rsa_key.d, rsa_key.n);
    std::cout << "  Original Message: " << message << "\n";
    std::cout << "  Encrypted Ciphertext: " << encrypted << "\n";
    std::cout << "  Decrypted Message: " << decrypted << "\n\n";

    std::cout << "========================================================\n\n";
    return 0;
}
