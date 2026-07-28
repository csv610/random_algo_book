// =============================================================================
// Naive Polynomial Multiplication (O(n*m) convolution)
//
// Given two polynomials A(x) = sum a_i x^i and B(x) = sum b_j x^j,
// compute their product C(x) = A(x)*B(x) by accumulating all pairwise
// coefficient products. All arithmetic is performed modulo a prime p.
//
// This is the baseline algorithm that Karatsuba and FFT-based methods
// improve upon for large-degree polynomials.
// =============================================================================

#include "ral/polynomial.h"
#include <iostream>
#include <vector>

namespace ral {

inline void demonstrate_polynomial_multiplication() {
    using Poly = Polynomial;
    const long long P = 97;

    std::cout << "=== Naive Polynomial Multiplication (mod " << P << ") ===\n\n";

    // Example 1: small polynomials
    Poly a({1, 2, 3});   // 1 + 2x + 3x^2
    Poly b({4, 5});      // 4 + 5x

    std::cout << "A(x)  = " << a.to_string() << "\n";
    std::cout << "B(x)  = " << b.to_string() << "\n";

    Poly c = a.multiply(b, P);
    std::cout << "A*B   = " << c.to_string() << "\n";
    std::cout << "Check : A(1)*B(1) mod " << P << " = "
              << (a.evaluate(1, P) * b.evaluate(1, P)) % P
              << ",  (A*B)(1) = " << c.evaluate(1, P) << "\n\n";

    // Example 2: squaring a polynomial
    Poly d({3, 1});      // 3 + x
    Poly sq = d.multiply(d, P);
    std::cout << "D(x)  = " << d.to_string() << "\n";
    std::cout << "D^2   = " << sq.to_string() << "\n\n";

    // Example 3: multiplying by (x - r) for all r in [0..4]
    // The product x(x-1)(x-2)(x-3)(x-4) has roots at 0,1,2,3,4
    Poly prod({1});
    for (long long r = 0; r < 5; ++r) {
        Poly factor({-r, 1});   // (x - r)
        prod = prod.multiply(factor, P);
    }
    std::cout << "x(x-1)(x-2)(x-3)(x-4) = " << prod.to_string() << "\n";
    std::cout << "Verify roots: ";
    for (long long r = 0; r < 5; ++r) {
        std::cout << "f(" << r << ")=" << prod.evaluate(r, P) << " ";
    }
    std::cout << "\n";
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_polynomial_multiplication();
    return 0;
}
