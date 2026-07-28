// example_quadratic_residue.cpp
//
// Quadratic Residues
// ====================
// An integer a is a quadratic residue (QR) mod p if there exists x such that:
//   x² ≡ a (mod p)
// Otherwise a is a quadratic non-residue (QNR).
//
// Euler's Criterion: a is QR mod odd prime p iff a^((p-1)/2) ≡ 1 (mod p)
//   - If a^((p-1)/2) ≡ 1 (mod p),  a is a QR
//   - If a^((p-1)/2) ≡ -1 (mod p), a is a QNR
//   - If a^((p-1)/2) ≡ 0 (mod p),  p divides a
//
// The Legendre symbol (a/p) encodes this:
//   (a/p) =  1  if a is QR mod p
//   (a/p) = -1  if a is QNR mod p
//   (a/p) =  0  if p | a
//
// For composite n, the Jacobi symbol (a/n) generalizes this.

#include "ral/crypto.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Quadratic Residues Demo\n";
    std::cout << "========================================================\n\n";

    // --- QR check for p = 13 ---
    std::cout << "--- Quadratic Residues mod 13 ---\n";
    {
        long long p = 13;
        std::cout << "  Testing each a in [0, " << (p - 1) << "]:\n\n";
        std::cout << "    a  | a^6 mod 13 | QR?   | Legendre\n";
        std::cout << "  ----+------------+-------+--------\n";

        for (long long a = 0; a < p; ++a) {
            long long euler = mod_pow(a, (p - 1) / 2, p);
            bool qr = is_quadratic_residue(a, p);
            int ls = legendre_symbol(a, p);
            std::cout << "    " << a << "  |     " << euler
                      << "      | " << (qr ? "yes" : "no ") << " |   "
                      << ls << "\n";
        }

        // List all QRs
        std::cout << "\n  Quadratic residues mod " << p << ": ";
        for (long long a = 1; a < p; ++a) {
            if (is_quadratic_residue(a, p)) {
                std::cout << a << " ";
            }
        }
        std::cout << "\n  (Note: there are always exactly (p-1)/2 QRs for prime p)\n";
    }

    // --- Multiple primes ---
    std::cout << "\n--- QR Summary for Various Primes ---\n";
    for (long long p : {7, 11, 13, 17, 19, 23}) {
        std::cout << "  QRs mod " << p << ": ";
        for (long long a = 1; a < p; ++a) {
            if (is_quadratic_residue(a, p)) {
                std::cout << a << " ";
            }
        }
        std::cout << "\n";
    }

    // --- Finding square roots via brute force ---
    std::cout << "\n--- Square Roots: Finding x such that x² ≡ a (mod p) ---\n";
    {
        long long p = 13, a = 3;
        std::cout << "  For a=" << a << ", p=" << p << ":\n";
        std::cout << "  Searching for x:\n";
        bool found = false;
        for (long long x = 0; x < p; ++x) {
            if ((x * x) % p == a) {
                std::cout << "    x = " << x << ": " << x << "² mod " << p
                          << " = " << (x * x) % p << "  ✓\n";
                found = true;
            }
        }
        if (!found) {
            std::cout << "    No solution exists (a is QNR)\n";
        }
    }

    return 0;
}
