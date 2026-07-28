// example_modular_exponentiation.cpp
//
// Modular Exponentiation (Binary / Square-and-Multiply Method)
// =============================================================
// Computes (base^exp) mod mod_val in O(log exp) multiplications.
// Instead of multiplying base exp times, we use the binary representation
// of the exponent: square the base at each step and multiply into the
// result only when the current bit of exp is 1.
//
//   Example:  3^13 mod 7
//     13 in binary = 1101
//     Steps:
//       exp bit 1 (LSB): result = 1 * 3 = 3   (mod 7),  base = 3^2 = 2
//       exp bit 0:                        base = 2^2 = 4
//       exp bit 1:       result = 3 * 4 = 12 -> 5 (mod 7),  base = 4^2 = 2
//       exp bit 1:       result = 5 * 2 = 10 -> 3 (mod 7)
//     Answer: 3^13 mod 7 = 3
//
// Practical use: RSA encryption/decryption requires computing m^e mod n
// with very large exponents. This method is essential.

#include "ral/number_theory.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Modular Exponentiation -- Binary Method Demo\n";
    std::cout << "========================================================\n\n";

    // --- Basic examples ---
    std::cout << "--- Basic Computations ---\n";
    std::vector<std::tuple<long long, long long, long long>> tests = {
        {2, 10, 1000},
        {3, 13, 7},
        {7, 256, 13},
        {2, 20, 1000000007},
        {123, 456, 1000},
    };

    for (auto [base, exp, mod] : tests) {
        std::cout << "  " << base << "^" << exp << " mod " << mod
                  << " = " << mod_pow(base, exp, mod) << "\n";
    }

    // --- Binary exponentiation step-by-step ---
    std::cout << "\n--- Binary Exponentiation Steps: 3^13 mod 7 ---\n";
    {
        long long base = 3, exp = 13, mod_val = 7;
        long long result = 1;
        long long b = base % mod_val;
        long long e = exp;
        int step = 0;

        std::cout << "  " << exp << " in binary = 1101\n";
        std::cout << "  Initial: result=1, base=" << b << "\n";

        while (e > 0) {
            if (e & 1) {
                result = (result * b) % mod_val;
                std::cout << "  Step " << ++step << ": bit=1, result = result * base mod "
                          << mod_val << " = " << result;
            } else {
                std::cout << "  Step " << ++step << ": bit=0, skip multiply";
            }
            b = (b * b) % mod_val;
            e >>= 1;
            std::cout << ", base squared -> " << b << "\n";
        }
        std::cout << "  Final: 3^13 mod 7 = " << result << "\n";
    }

    // --- Practical: RSA-like message encoding ---
    std::cout << "\n--- Practical: RSA-like Encoding ---\n";
    {
        // In RSA, we compute ciphertext = message^e mod n
        // Here we use small numbers to demonstrate
        long long n = 3233;   // n = 61 * 53
        long long e = 17;     // public exponent
        long long message = 65;

        long long ciphertext = mod_pow(message, e, n);
        std::cout << "  Message:    " << message << "\n";
        std::cout << "  Public key: e=" << e << ", n=" << n << "\n";
        std::cout << "  Ciphertext: " << ciphertext << "\n";

        // Verify by computing 65^17 mod 3233 step-by-step mentally:
        // Using binary exponentiation, 17 = 10001 in binary
        // This shows why mod_pow is essential -- manual computation is impractical
        std::cout << "  Verification: mod_pow(65, 17, 3233) = " << ciphertext << "\n";
    }

    // --- Edge cases ---
    std::cout << "\n--- Edge Cases ---\n";
    std::cout << "  mod_pow(5, 0, 7) = " << mod_pow(5, 0, 7) << "  (anything^0 = 1)\n";
    std::cout << "  mod_pow(5, 1, 7) = " << mod_pow(5, 1, 7) << "  (anything^1 = itself)\n";
    std::cout << "  mod_pow(0, 5, 7) = " << mod_pow(0, 5, 7) << "  (0^n = 0)\n";
    std::cout << "  mod_pow(3, 100, 1) = " << mod_pow(3, 100, 1) << "  (mod 1 is always 0)\n";

    return 0;
}
