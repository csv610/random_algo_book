// example_euclidean_gcd.cpp
//
// Euclidean Algorithm for GCD
// ============================
// The GCD (Greatest Common Divisor) of two integers is the largest
// positive integer that divides both. The Euclidean algorithm is based
// on the observation that gcd(a, b) = gcd(b, a mod b).
//
//   Example: gcd(48, 18)
//     48 = 2 * 18 + 12   -> gcd(18, 12)
//     18 = 1 * 12 + 6    -> gcd(12, 6)
//     12 = 2 * 6  + 0    -> gcd = 6
//
// Practical use: Simplifying fractions, computing LCM, checking if
// two numbers are coprime (needed for RSA key generation).

#include "ral/number_theory.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Euclidean GCD Algorithm Demo\n";
    std::cout << "========================================================\n\n";

    // --- Basic GCD computations ---
    std::cout << "--- GCD of Various Pairs ---\n";
    std::vector<std::pair<long long, long long>> pairs = {
        {48, 18}, {100, 75}, {17, 13}, {0, 5}, {1000, 250}
    };

    for (auto [a, b] : pairs) {
        std::cout << "  gcd(" << a << ", " << b << ") = " << gcd(a, b) << "\n";
    }

    // --- Step-by-step trace ---
    std::cout << "\n--- Step-by-Step: gcd(270, 192) ---\n";
    {
        long long a = 270, b = 192;
        int step = 0;
        while (b != 0) {
            long long r = a % b;
            std::cout << "  Step " << ++step << ": " << a << " = "
                      << (a / b) << " * " << b << " + " << r << "\n";
            a = b;
            b = r;
        }
        std::cout << "  GCD = " << a << "\n";
    }

    // --- Fraction simplification ---
    std::cout << "\n--- Fraction Simplification ---\n";
    struct Fraction { long long num, den; };
    std::vector<Fraction> fracs = {{12, 18}, {100, 250}, {7, 13}, {48, 36}};

    for (auto [num, den] : fracs) {
        long long g = gcd(num, den);
        std::cout << "  " << num << "/" << den << " = "
                  << (num / g) << "/" << (den / g)
                  << "  (divided both by " << g << ")\n";
    }

    // --- Coprimality check ---
    std::cout << "\n--- Coprimality Check (relevant for RSA) ---\n";
    std::vector<std::pair<long long, long long>> coprime_tests = {
        {17, 31}, {61, 53}, {15, 28}, {6, 10}
    };
    for (auto [a, b] : coprime_tests) {
        bool coprime = (gcd(a, b) == 1);
        std::cout << "  gcd(" << a << ", " << b << ") = " << gcd(a, b)
                  << "  -> " << (coprime ? "coprime" : "not coprime") << "\n";
    }

    // --- LCM via GCD ---
    std::cout << "\n--- LCM via GCD: lcm(a,b) = a*b / gcd(a,b) ---\n";
    for (auto [a, b] : pairs) {
        std::cout << "  lcm(" << a << ", " << b << ") = "
                  << (a / gcd(a, b)) * b << "\n";
    }

    return 0;
}
