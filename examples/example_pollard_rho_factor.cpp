// example_pollard_rho_factor.cpp
// Pollard's Rho: Finding a Single Non-Trivial Factor
//
// Uses Floyd's cycle detection on f(x) = x^2 + c (mod n) to find a
// non-trivial factor of a composite number n. Expected time: O(n^{1/4}).
// The algorithm exploits the birthday paradox: random values mod a
// factor p will collide after ~sqrt(p) steps.
//
// Compile: g++ -std=c++23 -I../include -o example_pollard_rho_factor example_pollard_rho_factor.cpp

#include <random>
#include <ral/pollard_rho.h>
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Pollard's Rho: Single Factor Finding ===\n\n";

    // Test numbers and their known factorizations
    std::vector<std::pair<long long, std::string>> tests = {
        {91,           "7 * 13"},
        {100,          "2^2 * 5^2"},
        {123456789LL,  "3^2 * 3607 * 3803"},
        {9801LL,       "3^4 * 11^2"},
        {10007LL * 10009LL, "10007 * 10009"},
    };

    std::cout << "Finding one non-trivial factor for each composite:\n\n";

    for (auto& [n, known] : tests) {
        if (is_prime_trial(n)) {
            std::cout << "  " << n << " is prime (skipped)\n";
            continue;
        }

        long long factor = pollard_rho_factor(n, rng);
        std::cout << "  " << n << "  ->  factor = " << factor
                  << "  (other factor = " << n / factor << ")"
                  << "  [known: " << known << "]\n";
    }

    // Demonstrate that it works on larger numbers too
    std::cout << "\nLarger composite numbers:\n";
    std::vector<long long> large = {
        1000000007LL * 2LL,   // 2 * 10^9+7
        999999937LL * 3LL,    // 3 * 999999937
        1234567891LL * 7LL,   // 7 * 1234567891
    };

    for (long long n : large) {
        long long factor = pollard_rho_factor(n, rng);
        std::cout << "  " << n << "  ->  factor = " << factor
                  << "  (other = " << n / factor << ")\n";
    }

    return 0;
}
