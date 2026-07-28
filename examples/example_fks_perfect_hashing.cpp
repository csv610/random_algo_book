// =====================================================================
// FKS Two-Level Perfect Hashing
//
// The Fredman-Komlos-Szemeredi (FKS) scheme achieves:
//   - O(n) total space
//   - O(1) worst-case lookup time
//
// Structure:
//   1. Primary hash: n buckets via universal hash
//   2. Secondary hash: for bucket of size m, allocate m^2 slots
//      and find a collision-free universal hash (expected O(1) trials)
//
// Total space = sum of m_i^2. By Markov's inequality, the expected
// total is O(n), giving constant amortized space per element.
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
    std::cout << "  FKS Two-Level Perfect Hashing\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);
    int n = 300;

    PerfectHashTable pht(n, rng);
    for (int i = 1; i <= n; i++)
        pht.insert(i * 7);  // insert spaced keys

    std::cout << "Inserted " << n << " keys (1*7, 2*7, ..., " << n << "*7)\n";
    std::cout << "  Table size (buckets):  " << n << "\n";
    std::cout << "  Total secondary space: " << pht.total_space() << "\n";
    std::cout << "  Space ratio:           " << std::fixed << std::setprecision(2)
              << (double)pht.total_space() / n << "x  (O(n) total)\n\n";

    // Search correctness
    std::cout << "--- Search correctness ---\n";
    int found = 0, not_found = 0;
    for (int i = 1; i <= n; i++) {
        if (pht.search(i * 7)) found++;
    }
    for (int i = n + 1; i <= n + 50; i++) {
        if (pht.search(i * 7)) not_found++;
    }
    std::cout << "  Present keys found:  " << found << "/" << n << "\n";
    std::cout << "  Absent keys found:   " << not_found << "/50  (should be 0)\n\n";

    // Remove and recheck
    pht.search(7 * 50);  // ensure it exists
    pht.remove(7 * 50);
    std::cout << "After removing key " << 7 * 50 << ":\n";
    std::cout << "  search(" << 7 * 50 << ") -> "
              << (pht.search(7 * 50) ? "FOUND" : "not found") << "\n\n";

    // --- Space scaling experiment ---
    std::cout << "--- Space Scaling ---\n";
    std::cout << std::setw(8) << "n"
              << std::setw(14) << "sec_space"
              << std::setw(12) << "ratio"
              << "\n";
    for (int sz : {100, 500, 1000, 3000}) {
        std::mt19937 r(sz);
        PerfectHashTable p(sz, r);
        for (int i = 1; i <= sz; i++)
            p.insert(i);
        std::cout << std::setw(8) << sz
                  << std::setw(14) << p.total_space()
                  << std::setw(12) << std::fixed << std::setprecision(2)
                  << (double)p.total_space() / sz << "x"
                  << "\n";
    }

    std::cout << "\nFKS guarantees O(n) total secondary space and O(1) worst-case lookup.\n";
    return 0;
}
