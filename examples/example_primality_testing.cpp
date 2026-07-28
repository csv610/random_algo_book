// example_primality_testing.cpp
//
// Trial Division Primality Test
// ===============================
// Tests whether n is prime by checking divisibility by 2, 3, then all
// numbers of the form 6k ± 1 up to sqrt(n). This works because every
// prime > 3 is of the form 6k ± 1.
//
// Time complexity: O(sqrt(n))
// Best for small numbers. For large numbers, probabilistic tests
// (Miller-Rabin, Solovay-Strassen) are preferred.

#include "ral/number_theory.h"
#include <iostream>
#include <vector>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Trial Division Primality Test Demo\n";
    std::cout << "========================================================\n\n";

    // --- Test a range of numbers ---
    std::cout << "--- Primality of Various Numbers ---\n";
    std::vector<long long> numbers = {
        0, 1, 2, 3, 4, 5, 13, 17, 25, 29, 97, 100, 997, 1000, 104729
    };

    for (long long n : numbers) {
        std::cout << "  is_prime(" << n << ") = "
                  << (is_prime_trial(n) ? "true" : "false") << "\n";
    }

    // --- Find primes in a range ---
    std::cout << "\n--- Primes from 1 to 50 ---\n";
    std::cout << "  ";
    int count = 0;
    for (long long n = 1; n <= 50; ++n) {
        if (is_prime_trial(n)) {
            std::cout << n << " ";
            ++count;
        }
    }
    std::cout << "\n  Count: " << count << " primes\n";

    // --- How trial division works step-by-step ---
    std::cout << "\n--- Trial Division Steps for n = 97 ---\n";
    {
        long long n = 97;
        std::cout << "  Check 97 < 2? No\n";
        std::cout << "  Check 97 == 2 or 3? No\n";
        std::cout << "  Check 97 % 2 == 0? No\n";
        std::cout << "  Check 97 % 3 == 0? No\n";
        std::cout << "  Trial divide by 6k±1 form:\n";
        for (long long i = 5; i * i <= n; i += 6) {
            std::cout << "    97 % " << i << " = " << (n % i)
                      << ", 97 % " << (i + 2) << " = " << (n % (i + 2)) << "\n";
        }
        std::cout << "  No divisors found -> 97 is prime\n";
    }

    // --- Counting primes ---
    std::cout << "\n--- Prime Counting: π(n) ---\n";
    std::vector<long long> limits = {10, 50, 100, 500, 1000};
    for (long long limit : limits) {
        int prime_count = 0;
        for (long long n = 2; n <= limit; ++n) {
            if (is_prime_trial(n)) ++prime_count;
        }
        std::cout << "  π(" << limit << ") = " << prime_count << "\n";
    }

    // --- Large prime check ---
    std::cout << "\n--- Checking Larger Numbers ---\n";
    std::vector<long long> large_nums = {100003, 100043, 999983, 1000003};
    for (long long n : large_nums) {
        std::cout << "  is_prime(" << n << ") = "
                  << (is_prime_trial(n) ? "true" : "false") << "\n";
    }

    return 0;
}
