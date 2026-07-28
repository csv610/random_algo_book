// example_legendre_symbol.cpp
//
// Legendre Symbol (a / p)
// ========================
// For an odd prime p, the Legendre symbol is defined as:
//   (a/p) =  1  if a is a quadratic residue mod p (a ≠ 0, x²≡a has solution)
//   (a/p) = -1  if a is a quadratic non-residue mod p
//   (a/p) =  0  if p divides a
//
// Computed via Euler's criterion: (a/p) ≡ a^((p-1)/2) (mod p)
//
// Properties:
//   - (a/p) = (b/p) if a ≡ b (mod p)
//   - (ab/p) = (a/p)(b/p)
//   - (1/p) = 1, (-1/p) = (-1)^((p-1)/2)
//   - (2/p) = (-1)^((p²-1)/8)
//   - Quadratic Reciprocity: for distinct odd primes p,q:
//     (p/q)(q/p) = (-1)^((p-1)(q-1)/4)

#include "ral/crypto.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Legendre Symbol Demo\n";
    std::cout << "========================================================\n\n";

    // --- Legendre symbol for p = 7 ---
    std::cout << "--- Legendre Symbol (a / 7) ---\n";
    for (long long a = 0; a <= 6; ++a) {
        int ls = legendre_symbol(a, 7);
        std::cout << "  (" << a << " / 7) = " << ls;
        if (ls == 1) std::cout << "  (QR)";
        else if (ls == -1) std::cout << " (QNR)";
        else std::cout << "  (divisible)";
        std::cout << "\n";
    }

    // --- Full table for several primes ---
    std::cout << "\n--- Legendre Symbol Tables ---\n";
    for (long long p : {7, 11, 13, 17, 19, 23}) {
        std::cout << "\n  p = " << p << ":\n";
        std::cout << "  a | (a/" << p << ")\n";
        std::cout << "  --+-------\n";
        for (long long a = 0; a < p; ++a) {
            std::cout << "  " << a << " |   " << legendre_symbol(a, p) << "\n";
        }
    }

    // --- Verification of properties ---
    std::cout << "\n--- Property Verification ---\n";

    // Multiplicativity: (ab/p) = (a/p)(b/p)
    {
        long long p = 13;
        std::cout << "  Multiplicativity on p=" << p << ":\n";
        std::cout << "  (2/" << p << ")*(3/" << p << ") = "
                  << legendre_symbol(2, p) * legendre_symbol(3, p) << "\n";
        std::cout << "  (6/" << p << ")         = "
                  << legendre_symbol(6, p) << "\n";
    }

    // (-1/p) = (-1)^((p-1)/2)
    std::cout << "\n  (-1/p) = (-1)^((p-1)/2):\n";
    for (long long p : {7, 11, 13, 17}) {
        int ls = legendre_symbol(-1, p);
        int expected = ((p - 1) / 2 % 2 == 0) ? 1 : -1;
        std::cout << "    (" << -1 << "/" << p << ") = " << ls
                  << " (expected " << expected << ")\n";
    }

    // (2/p) = (-1)^((p²-1)/8)
    std::cout << "\n  (2/p) = (-1)^((p²-1)/8):\n";
    for (long long p : {7, 11, 17, 23}) {
        int ls = legendre_symbol(2, p);
        int expected = (((p * p - 1) / 8) % 2 == 0) ? 1 : -1;
        std::cout << "    (2/" << p << ") = " << ls
                  << " (expected " << expected << ")\n";
    }

    return 0;
}
