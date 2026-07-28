// =============================================================================
// Horner's Method for Polynomial Evaluation
//
// Evaluate p(x) = a_0 + a_1 x + a_2 x^2 + ... + a_n x^n at a point x
// using the nested form:
//   p(x) = a_0 + x(a_1 + x(a_2 + ... + x * a_n)...)
//
// This reduces n multiplications and n additions — optimal for sequential
// evaluation. The algorithm processes coefficients from highest degree
// downward, accumulating: result = result * x + a_i.
// =============================================================================

#include "ral/polynomial.h"
#include "ral/polynomial_chapter8.h"
#include <iostream>
#include <vector>

namespace ral {

inline void demonstrate_horner_evaluation() {
    std::cout << "=== Horner's Method for Polynomial Evaluation ===\n\n";

    const long long MOD = 1000000007LL;

    // Polynomial: p(x) = 3 + 0x + 2x^2 + x^3  => coeffs = {3, 0, 2, 1}
    std::vector<int> coeffs = {3, 0, 2, 1};
    long long x = 5;

    std::cout << "p(x) = 3 + 2x^2 + x^3\n";
    std::cout << "x = " << x << "\n\n";

    // Direct evaluation (verification)
    long long direct = 3 + 2LL * x * x + 1LL * x * x * x;
    std::cout << "Direct evaluation:  p(" << x << ") = " << direct << "\n";

    // Horner via polynomial_chapter8
    long long horner_val = ral::poly_eval(coeffs, x, MOD);
    std::cout << "Horner (mod " << MOD << "): p(" << x << ") = " << horner_val << "\n";

    // Horner via Polynomial class (polynomial.h)
    ral::Polynomial poly(std::vector<long long>(coeffs.begin(), coeffs.end()));
    long long horner2 = poly.evaluate(x, MOD);
    std::cout << "Polynomial::eval:   p(" << x << ") = " << horner2 << "\n\n";

    // Larger polynomial: p(x) = 1 + x + x^2 + ... + x^10
    std::vector<int> geometric(11, 1);
    std::cout << "Geometric series: p(x) = 1 + x + x^2 + ... + x^10\n";
    std::cout << "p(2) mod " << MOD << " = " << ral::poly_eval(geometric, 2, MOD) << "\n";
    std::cout << "Expected:          " << ((1LL << 11) - 1) % MOD << "\n\n";

    // Step-by-step Horner trace for p(x) = 3 + 2x^2 + x^3 at x = 5
    std::cout << "Step-by-step Horner trace for p(5):\n";
    long long result = 0;
    // coeffs high-to-low: 1, 2, 0, 3
    int hi_coeffs[] = {1, 2, 0, 3};
    for (int i = 0; i < 4; ++i) {
        result = result * 5 + hi_coeffs[i];
        std::cout << "  step " << i << ": result = " << result << "\n";
    }
    std::cout << "  final: " << result << "\n";
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_horner_evaluation();
    return 0;
}
