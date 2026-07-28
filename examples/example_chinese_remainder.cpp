// example_chinese_remainder.cpp
//
// Chinese Remainder Theorem (CRT)
// =================================
// Given a system of congruences with pairwise coprime moduli:
//   x ≡ a_1 (mod n_1)
//   x ≡ a_2 (mod n_2)
//   ...
//   x ≡ a_k (mod n_k)
//
// CRT guarantees a unique solution x modulo N = n_1 * n_2 * ... * n_k.
//
// Construction:
//   M = product of all n_i
//   M_i = M / n_i
//   y_i = M_i^(-1) mod n_i
//   x = sum(a_i * M_i * y_i) mod M
//
// Practical uses:
//   - Secret sharing (splitting secrets across residues)
//   - Parallel computation with modular arithmetic
//   - RSA-CRT optimization for decryption

#include "ral/number_theory.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Chinese Remainder Theorem Demo\n";
    std::cout << "========================================================\n\n";

    // --- Classic problem ---
    std::cout << "--- Classic Problem ---\n";
    std::cout << "  \"What number leaves remainder 2 when divided by 3,\n";
    std::cout << "   remainder 3 when divided by 5, and\n";
    std::cout << "   remainder 2 when divided by 7?\"\n\n";

    {
        std::vector<Congruence> system = {{2, 3}, {3, 5}, {2, 7}};
        long long x = chinese_remainder_theorem(system);
        std::cout << "  x ≡ 2 (mod 3)\n";
        std::cout << "  x ≡ 3 (mod 5)\n";
        std::cout << "  x ≡ 2 (mod 7)\n";
        std::cout << "  Solution: x = " << x << " (mod " << 3 * 5 * 7 << ")\n";
        std::cout << "  Verify: " << x << " mod 3 = " << x % 3
                  << ", " << x << " mod 5 = " << x % 5
                  << ", " << x << " mod 7 = " << x % 7 << "\n";
    }

    // --- Step-by-step construction ---
    std::cout << "\n--- Step-by-Step Construction ---\n";
    {
        std::vector<Congruence> system = {{2, 3}, {3, 5}, {2, 7}};
        long long M = 3 * 5 * 7;
        std::cout << "  M = 3 * 5 * 7 = " << M << "\n\n";

        for (int i = 0; i < 3; ++i) {
            long long n_i = system[i].n;
            long long a_i = system[i].a;
            long long M_i = M / n_i;
            long long y_i = mod_inverse(M_i, n_i);
            std::cout << "  Congruence " << (i + 1) << ": x ≡ " << a_i << " (mod " << n_i << ")\n";
            std::cout << "    M_" << (i + 1) << " = " << M << " / " << n_i << " = " << M_i << "\n";
            std::cout << "    y_" << (i + 1) << " = " << M_i << "^(-1) mod " << n_i << " = " << y_i << "\n";
            std::cout << "    term = " << a_i << " * " << M_i << " * " << y_i
                      << " = " << (a_i * M_i * y_i) << "\n\n";
        }
    }

    // --- Another example ---
    std::cout << "--- Example: Days of the Week ---\n";
    std::cout << "  \"A cycle repeats every N days. On day x:\n";
    std::cout << "   it's day 1 in a 3-day cycle,\n";
    std::cout << "   day 2 in a 5-day cycle, and\n";
    std::cout << "   day 0 in a 7-day cycle.\"\n";
    {
        std::vector<Congruence> system = {{1, 3}, {2, 5}, {0, 7}};
        long long x = chinese_remainder_theorem(system);
        std::cout << "  Solution: x = " << x << " (mod " << 3 * 5 * 7 << ")\n";
        std::cout << "  Verify: " << x << " mod 3 = " << x % 3
                  << ", " << x << " mod 5 = " << x % 5
                  << ", " << x << " mod 7 = " << x % 7 << "\n";
    }

    // --- Larger system ---
    std::cout << "\n--- Larger System ---\n";
    {
        std::vector<Congruence> system = {
            {1, 3}, {2, 5}, {3, 7}, {4, 11}, {5, 13}
        };
        long long M = 1;
        for (auto& c : system) M *= c.n;

        long long x = chinese_remainder_theorem(system);
        std::cout << "  System: x≡1(mod 3), x≡2(mod 5), x≡3(mod 7),\n";
        std::cout << "          x≡4(mod 11), x≡5(mod 13)\n";
        std::cout << "  Solution: x = " << x << " (mod " << M << ")\n";
        std::cout << "  Verify:\n";
        for (auto& c : system) {
            std::cout << "    " << x << " mod " << c.n << " = " << (x % c.n)
                      << " (expected " << c.a << ")\n";
        }
    }

    return 0;
}
