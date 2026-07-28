// example_jacobi_symbol.cpp
//
// Jacobi Symbol (a / n)
// =======================
// Generalization of the Legendre symbol to composite (odd) moduli.
// For n = p1^e1 * p2^e2 * ... * pk^ek:
//   (a/n) = (a/p1)^e1 * (a/p2)^e2 * ... * (a/pk)^ek
//
// Unlike the Legendre symbol, (a/n) = 1 does NOT guarantee that a is a
// QR mod n. For example, (2/15) = 1 but 2 is not a QR mod 15.
//
// Computed efficiently using:
//   - Factor out powers of 2 from a: (2^v * a' / n) with sign rule
//   - Quadratic reciprocity swap: (a/n) → (n/a) * (-1)^((a-1)(n-1)/4)
//
// Practical uses:
//   - Solovay-Strassen primality test uses Jacobi symbol
//   - Quadratic residue testing for composite moduli

#include "ral/crypto.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Jacobi Symbol Demo\n";
    std::cout << "========================================================\n\n";

    // --- Jacobi symbol for composite n = 15 ---
    std::cout << "--- Jacobi Symbol (a / 15), where 15 = 3 × 5 ---\n";
    for (long long a = 1; a < 15; ++a) {
        int js = jacobi_symbol(a, 15);
        std::cout << "  (" << a << " / 15) = " << js << "\n";
    }

    // --- Compare Legendre vs Jacobi ---
    std::cout << "\n--- Legendre vs Jacobi ---\n";
    std::cout << "  For prime p, Jacobi = Legendre:\n";
    for (long long a = 1; a < 7; ++a) {
        int ls = legendre_symbol(a, 7);
        int js = jacobi_symbol(a, 7);
        std::cout << "    (" << a << "/7): Legendre=" << ls
                  << ", Jacobi=" << js
                  << (ls == js ? "  match" : "  DIFFER") << "\n";
    }

    std::cout << "\n  For composite n, Jacobi ≠ Legendre (Legendre not defined):\n";
    std::cout << "  Jacobi(2, 15) = " << jacobi_symbol(2, 15)
              << "  (but 2 is NOT a QR mod 15)\n";

    // --- More composite moduli ---
    std::cout << "\n--- Jacobi Symbol for Various Composite n ---\n";
    for (long long n : {9, 15, 21, 33, 35}) {
        std::cout << "\n  n = " << n << ":\n";
        std::cout << "  a | (a/" << n << ")\n";
        std::cout << "  --+-------\n";
        for (long long a = 1; a < n; ++a) {
            int js = jacobi_symbol(a, n);
            std::cout << "  " << a << " |   " << js << "\n";
        }
    }

    // --- Jacobi symbol properties ---
    std::cout << "\n--- Properties ---\n";
    std::cout << "  (a/n) ∈ {-1, 0, 1} always\n";
    std::cout << "  (0/n) = 0\n";
    std::cout << "  (1/n) = 1\n";
    std::cout << "  (a/n) = (b/n) if a ≡ b (mod n)\n";

    // Verify multiplicativity doesn't hold in general for Jacobi
    std::cout << "\n  Note: (a/n)*(b/n) = (ab/n) still holds for Jacobi.\n";
    std::cout << "  Verify with n=15: (2/15)*(3/15) = "
              << jacobi_symbol(2, 15) * jacobi_symbol(3, 15)
              << ", (6/15) = " << jacobi_symbol(6, 15) << "\n";

    return 0;
}
