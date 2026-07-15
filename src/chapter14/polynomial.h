#pragma once

#include "number_theory.h"

#include <print>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>

namespace randalgo {

// ---------------------------------------------------------------------------
// Polynomial class -- coefficients stored as long long, arithmetic mod p.
//   Coefficients are ordered from lowest degree to highest:
//     coeffs[0] = constant term, coeffs[1] = x^1, ...
// ---------------------------------------------------------------------------
class Polynomial {
public:
    std::vector<long long> coeffs;

    Polynomial() = default;

    explicit Polynomial(std::vector<long long> c) : coeffs(std::move(c)) {
        trim();
    }

    // Evaluate at x modulo p using Horner's method: O(n)
    long long evaluate(long long x, long long p) const {
        long long result = 0;
        for (auto it = coeffs.rbegin(); it != coeffs.rend(); ++it) {
            result = (result * x + *it) % p;
        }
        return ((result % p) + p) % p;
    }

    // Polynomial addition mod p
    Polynomial add(const Polynomial& other, long long p) const {
        size_t max_len = std::max(coeffs.size(), other.coeffs.size());
        std::vector<long long> result(max_len, 0);
        for (size_t i = 0; i < max_len; ++i) {
            long long a = (i < coeffs.size()) ? coeffs[i] : 0;
            long long b = (i < other.coeffs.size()) ? other.coeffs[i] : 0;
            result[i] = ((a + b) % p + p) % p;
        }
        return Polynomial(std::move(result));
    }

    // Polynomial multiplication mod p -- O(n * m)
    Polynomial multiply(const Polynomial& other, long long p) const {
        if (coeffs.empty() || other.coeffs.empty()) return Polynomial();
        std::vector<long long> result(coeffs.size() + other.coeffs.size() - 1, 0);
        for (size_t i = 0; i < coeffs.size(); ++i) {
            for (size_t j = 0; j < other.coeffs.size(); ++j) {
                result[i + j] = (result[i + j] + coeffs[i] * other.coeffs[j]) % p;
            }
        }
        for (auto& c : result) c = ((c % p) + p) % p;
        return Polynomial(std::move(result));
    }

    // Subtract another polynomial mod p
    Polynomial subtract(const Polynomial& other, long long p) const {
        size_t max_len = std::max(coeffs.size(), other.coeffs.size());
        std::vector<long long> result(max_len, 0);
        for (size_t i = 0; i < max_len; ++i) {
            long long a = (i < coeffs.size()) ? coeffs[i] : 0;
            long long b = (i < other.coeffs.size()) ? other.coeffs[i] : 0;
            result[i] = ((a - b) % p + p) % p;
        }
        return Polynomial(std::move(result));
    }

    bool is_zero() const {
        for (long long c : coeffs) {
            if (c != 0) return false;
        }
        return true;
    }

    size_t degree() const {
        if (coeffs.empty()) return 0;
        for (size_t i = coeffs.size(); i-- > 0;) {
            if (coeffs[i] != 0) return i;
        }
        return 0;
    }

    std::string to_string() const {
        if (coeffs.empty()) return "0";
        std::string s;
        for (int i = static_cast<int>(coeffs.size()) - 1; i >= 0; --i) {
            if (coeffs[i] == 0) continue;
            if (!s.empty()) s += " + ";
            if (i == 0 || coeffs[i] != 1) {
                s += std::to_string(coeffs[i]);
            }
            if (i > 0) {
                s += "x";
                if (i > 1) s += "^" + std::to_string(i);
            }
        }
        return s.empty() ? "0" : s;
    }

private:
    void trim() {
        while (coeffs.size() > 1 && coeffs.back() == 0) {
            coeffs.pop_back();
        }
    }
};

// Shared PRNG for polynomial random evaluation
static inline std::mt19937_64& poly_rng() {
    static std::mt19937_64 rng{
        static_cast<unsigned>(
            std::chrono::steady_clock::now().time_since_epoch().count())};
    return rng;
}

// ---------------------------------------------------------------------------
// Schwartz-Zippel Test
//   If a polynomial over a field is identically zero, then Pr[f(r)=0] = 1
//   for random r. If f != 0 with degree <= d over field Z_p, then
//   Pr[f(r)=0] <= d/p. Repeated trials give high confidence.
// ---------------------------------------------------------------------------
bool schwartz_zippel_test(const Polynomial& poly, long long p, int num_trials = 20) {
    if (poly.is_zero()) return true;

    std::uniform_int_distribution<long long> dist(0, p - 1);

    for (int i = 0; i < num_trials; ++i) {
        long long r = dist(poly_rng());
        if (poly.evaluate(r, p) != 0) {
            return false;  // Found a non-root -> polynomial is not identically zero
        }
    }
    return true;  // All trials returned zero -- very likely identically zero
}

// ---------------------------------------------------------------------------
// Polynomial Identity Testing
//   Tests whether poly1 == poly2 (mod p) by checking if poly1 - poly2
//   is identically zero using Schwartz-Zippel.
// ---------------------------------------------------------------------------
bool polynomial_identity_test(const Polynomial& poly1,
                               const Polynomial& poly2,
                               long long p,
                               int num_trials = 20) {
    Polynomial diff = poly1.subtract(poly2, p);
    return schwartz_zippel_test(diff, p, num_trials);
}

// ---------------------------------------------------------------------------
// Demonstration
// ---------------------------------------------------------------------------
void demonstrate_polynomial() {
    std::println("=== Polynomial Operations over Finite Fields ===\n");

    long long p = 97;  // A small prime

    // --- Basic polynomial operations ---
    std::println("--- Basic Operations (mod {}) ---", p);
    {
        Polynomial f({1, 2, 3});       // 3x^2 + 2x + 1
        Polynomial g({4, 5});          // 5x + 4

        std::println("  f(x) = {}", f.to_string());
        std::println("  g(x) = {}", g.to_string());

        Polynomial sum = f.add(g, p);
        std::println("  f + g = {}", sum.to_string());

        Polynomial diff = f.subtract(g, p);
        std::println("  f - g = {}", diff.to_string());

        Polynomial prod = f.multiply(g, p);
        std::println("  f * g = {}", prod.to_string());

        // Evaluate at x = 10 mod 97
        long long x = 10;
        std::println("  f({}) mod {} = {}", x, p, f.evaluate(x, p));
        std::println("  g({}) mod {} = {}", x, p, g.evaluate(x, p));
        std::println("  (f*g)({}) mod {} = {}", x, p, prod.evaluate(x, p));
    }

    // --- Schwartz-Zippel: zero polynomial ---
    std::println("\n--- Schwartz-Zippel: Zero Polynomial ---");
    {
        Polynomial zero({0, 0, 0, 0});  // identically zero
        bool result = schwartz_zippel_test(zero, p, 30);
        std::println("  Polynomial: 0 (all coefficients zero)");
        std::println("  Schwartz-Zippel says identically zero? {}", result ? "YES" : "NO");
    }

    // --- Schwartz-Zippel: non-zero polynomial that happens to vanish at some points ---
    std::println("\n--- Schwartz-Zippel: Non-Zero Polynomial ---");
    {
        // f(x) = x^2 - x = x(x-1) -- vanishes at x=0 and x=1 but is NOT identically zero
        Polynomial f({0, -1, 1});
        std::println("  Polynomial: {} = x(x-1)", f.to_string());
        std::println("  f(0) = {}, f(1) = {}, f(2) = {}",
                      f.evaluate(0, p), f.evaluate(1, p), f.evaluate(2, p));
        bool result = schwartz_zippel_test(f, p, 30);
        std::println("  Schwartz-Zippel says identically zero? {}", result ? "YES" : "NO");
    }

    // --- Schwartz-Zippel: a "tall" polynomial that vanishes on many points ---
    std::println("\n--- Schwartz-Zippel: Product of Many Roots ---");
    {
        // f(x) = x(x-1)(x-2)...(x-9) -- vanishes at 10 points but degree 10 << 97
        Polynomial f({1});  // start with constant 1
        Polynomial factor({0, 1});  // x
        for (long long r = 0; r < 10; ++r) {
            Polynomial shift({-r, 1});  // (x - r)
            f = f.multiply(shift, p);
        }
        std::println("  Polynomial degree: {}", f.degree());
        std::println("  Schwartz-Zippel says identically zero? {}",
                      schwartz_zippel_test(f, p, 30) ? "YES" : "NO");
    }

    // --- Polynomial Identity Testing ---
    std::println("\n--- Polynomial Identity Testing ---");
    {
        Polynomial a({3, 2, 1});   // x^2 + 2x + 3
        Polynomial b({3, 2, 1});   // same polynomial
        Polynomial c({1, 2, 3});   // different: 3x^2 + 2x + 1

        std::println("  a(x) = {}", a.to_string());
        std::println("  b(x) = {}", b.to_string());
        std::println("  c(x) = {}", c.to_string());
        std::println("  a == b? {}", polynomial_identity_test(a, b, p, 30) ? "YES" : "NO");
        std::println("  a == c? {}", polynomial_identity_test(a, c, p, 30) ? "YES" : "NO");
    }

    // --- Schwartz-Zippel probability bound ---
    std::println("\n--- Schwartz-Zippel Probability Bound ---");
    {
        // For degree-d poly over Z_p, Pr[f(r)=0 | f!=0] <= d/p
        long long p_val = 97;
        std::println("  Over Z_{} (p = {}):", p_val, p_val);
        std::println("    degree 1:  error <= {:.4f}", 1.0 / p_val);
        std::println("    degree 5:  error <= {:.4f}", 5.0 / p_val);
        std::println("    degree 10: error <= {:.4f}", 10.0 / p_val);
        std::println("    degree 50: error <= {:.4f}", 50.0 / p_val);
        std::println("  With 20 trials, degree 10: error <= {:.2e}", std::pow(10.0 / p_val, 20));
    }
}

} // namespace randalgo
