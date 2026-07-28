// =====================================================================
// Universal Hashing with Chaining
//
// Universal hashing selects a hash function at random from a family
//   h(x) = ((a*x + b) mod p) mod m
// where p is a large prime and a != 0, b are random.
//
// Key properties:
//   - Expected O(1) lookup, insert, and delete per operation
//   - For any pair x != y,  Pr[h(x) = h(y)] <= 1/m
//   - Average chain length = load factor alpha = n/m
//
// This example inserts n keys into a table of size m = n,
// then measures chain statistics and compares with theory.
// =====================================================================

#include "ral/hash_table.h"
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Universal Hashing with Chaining\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);
    int n = 200;

    // --- Basic insert / search / remove demo ---
    UniversalHashTable ht(n, rng);
    for (int i = 1; i <= n; i++)
        ht.insert(i);

    std::cout << "Inserted keys 1.." << n << " into table of size " << n << "\n";
    std::cout << "  Load factor:  " << std::fixed << std::setprecision(2) << ht.load_factor() << "\n";
    std::cout << "  Avg chain:    " << std::fixed << std::setprecision(2) << ht.avg_chain_length() << "\n";
    std::cout << "  Max chain:    " << ht.max_chain_length() << "\n\n";

    // Search demos
    std::cout << "search(50)  -> " << (ht.search(50)  ? "FOUND" : "not found") << "\n";
    std::cout << "search(999) -> " << (ht.search(999) ? "FOUND" : "not found") << "\n";

    ht.remove(50);
    std::cout << "after remove(50):\n";
    std::cout << "  search(50) -> " << (ht.search(50) ? "FOUND" : "not found") << "\n\n";

    // --- Collision analysis across table sizes ---
    std::cout << "--- Collision Analysis (1000 trials each) ---\n";
    std::cout << std::setw(8) << "n"
              << std::setw(12) << "avg_chain"
              << std::setw(12) << "avg_max"
              << std::setw(12) << "lg(lg n)"
              << "\n";

    for (int n : {100, 500, 1000, 5000}) {
        double total_avg = 0, total_max = 0;
        constexpr int trials = 1000;
        for (int t = 0; t < trials; t++) {
            std::mt19937 trial_rng(t * 77777 + n);
            UniversalHashTable ht2(n, trial_rng);
            for (int i = 1; i <= n; i++)
                ht2.insert(i);
            total_avg += ht2.avg_chain_length();
            total_max += ht2.max_chain_length();
        }
        double expected_max = std::log(n) / std::log(std::log(n));
        std::cout << std::setw(8) << n
                  << std::setw(12) << std::fixed << std::setprecision(2) << total_avg / trials
                  << std::setw(12) << std::fixed << std::setprecision(1) << total_max / trials
                  << std::setw(12) << std::fixed << std::setprecision(1) << expected_max
                  << "\n";
    }

    std::cout << "\nTheory: avg chain = alpha = n/m = 1.00, max chain ~ O(lg n / lg lg n)\n";
    return 0;
}
