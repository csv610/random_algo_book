// =============================================================================
// Schwartz-Zippel Polynomial Identity Testing (PIT)
//
// Given two polynomials p(x) and q(x) over a field Z_p, test whether
// p == q by checking if f(x) = p(x) - q(x) is identically zero.
//
// Key insight: a non-zero polynomial of degree d over Z_p has at most d
// roots, so for a random r in Z_p: Pr[f(r) = 0 | f != 0] <= d/p.
// Repeating t trials gives error probability (d/p)^t, which is
// exponentially small for large p.
//
// This gives a BPP algorithm for the polynomial identity problem.
// =============================================================================

#include "ral/polynomial.h"
#include "ral/polynomial_chapter8.h"
#include <iostream>
#include <vector>
#include <random>

namespace ral {

inline void demonstrate_schwartz_zippel() {
    std::mt19937 rng(42);
    const long long P = 97;
    const int TRIALS = 20;

    std::cout << "=== Schwartz-Zippel Polynomial Identity Testing ===\n";
    std::cout << "Field: Z_" << P << ",  Trials: " << TRIALS << "\n\n";

    // Test 1: Identical polynomials — should return true
    std::vector<int> p1 = {1, 2, 1};  // x^2 + 2x + 1
    std::vector<int> q1 = {1, 2, 1};
    bool eq = ral::polynomial_identity_test(p1, q1, P, TRIALS, rng);
    std::cout << "Test 1: x^2+2x+1 == x^2+2x+1  =>  "
              << (eq ? "IDENTICAL (correct)" : "DIFFERENT (wrong)") << "\n\n";

    // Test 2: Different polynomials — should return false
    std::vector<int> p2 = {1, 2, 1};
    std::vector<int> q2 = {1, 3, 1};  // different linear coeff
    eq = ral::polynomial_identity_test(p2, q2, P, TRIALS, rng);
    std::cout << "Test 2: x^2+2x+1 == x^2+3x+1  =>  "
              << (eq ? "IDENTICAL (wrong)" : "DIFFERENT (correct)") << "\n\n";

    // Test 3: Algebraic identity — (x+1)^2 == x^2+2x+1
    std::vector<int> left  = {1, 2, 1};   // (x+1)^2 expanded
    std::vector<int> right = {1, 2, 1};
    eq = ral::polynomial_identity_test(left, right, P, TRIALS, rng);
    std::cout << "Test 3: (x+1)^2 == x^2+2x+1  =>  "
              << (eq ? "IDENTICAL (correct)" : "DIFFERENT (wrong)") << "\n\n";

    // Test 4: Degree-5 polynomial that is NOT zero
    // p(x) = x(x-1)(x-2)(x-3)(x-4) — vanishes at 0..4 but NOT identically zero
    ral::Polynomial f({1});
    for (long long r = 0; r < 5; ++r) {
        f = f.multiply(ral::Polynomial({-r, 1}), P);
    }
    std::cout << "Test 4: f(x) = x(x-1)(x-2)(x-3)(x-4), degree " << f.degree() << "\n";
    std::cout << "  f(0)=" << f.evaluate(0, P) << " f(1)=" << f.evaluate(1, P)
              << " f(5)=" << f.evaluate(5, P) << "\n";

    ral::Polynomial zero({0});
    bool is_zero = ral::schwartz_zippel_test(f, P, TRIALS);
    std::cout << "  Is f identically zero?  " << (is_zero ? "YES (wrong)" : "NO (correct)") << "\n\n";

    // Error probability analysis
    std::cout << "Error probability analysis (degree d, field size " << P << "):\n";
    for (int d : {1, 5, 10, 50}) {
        double per_trial = static_cast<double>(d) / P;
        double total = std::pow(per_trial, TRIALS);
        std::cout << "  d=" << d << ": per-trial error <= " << per_trial
                  << ",  after " << TRIALS << " trials <= " << total << "\n";
    }
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_schwartz_zippel();
    return 0;
}
