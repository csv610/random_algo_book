// =====================================================================
// Two-Level Hashing Scheme
//
// Two-level hashing uses a first-level hash to distribute keys into
// buckets, then a second-level hash within each bucket. Combined with
// universal hashing at both levels, it achieves:
//   - O(1) expected lookup time
//   - O(n) total space
//   - Simple implementation vs FKS perfect hashing
//
// Key insight: if the first level creates buckets of size m, the
// second level uses a table of size O(m). The expected total space
// across all second-level tables is O(n) by linearity of expectation.
//
// This example compares two-level hashing with standard universal
// hashing, measuring chain lengths and lookup performance.
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
    std::cout << "  Two-Level Hashing Scheme\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);
    int n = 500;

    // Build a two-level hash using the PerfectHashTable (which is
    // exactly the FKS two-level scheme: primary universal hash into
    // buckets, secondary universal hash within each bucket)
    PerfectHashTable twolevel(n, rng);
    for (int i = 1; i <= n; i++)
        twolevel.insert(i);

    // Also build a standard universal hash table for comparison
    UniversalHashTable single_level(n, rng);
    for (int i = 1; i <= n; i++)
        single_level.insert(i);

    std::cout << "n = " << n << " elements\n\n";

    std::cout << "--- Structure Comparison ---\n";
    std::cout << "  Single-level (universal chaining):\n";
    std::cout << "    Table size:    " << single_level.table_size() << "\n";
    std::cout << "    Load factor:   " << std::fixed << std::setprecision(2)
              << single_level.load_factor() << "\n";
    std::cout << "    Avg chain:     " << std::fixed << std::setprecision(2)
              << single_level.avg_chain_length() << "\n";
    std::cout << "    Max chain:     " << single_level.max_chain_length() << "\n\n";

    std::cout << "  Two-level (FKS):\n";
    std::cout << "    Primary buckets: " << n << "\n";
    std::cout << "    Total secondary: " << twolevel.total_space() << " slots\n";
    std::cout << "    Space ratio:     " << std::fixed << std::setprecision(2)
              << (double)twolevel.total_space() / n << "x\n";
    std::cout << "    Lookup:          O(1) worst-case\n\n";

    // --- Search correctness ---
    std::cout << "--- Search Correctness ---\n";
    int sl_found = 0, tl_found = 0;
    for (int i = 1; i <= n; i++) {
        if (single_level.search(i)) sl_found++;
        if (twolevel.search(i)) tl_found++;
    }
    std::cout << "  Single-level found: " << sl_found << "/" << n << "\n";
    std::cout << "  Two-level found:    " << tl_found << "/" << n << "\n\n";

    // Absent keys
    int sl_absent = 0, tl_absent = 0;
    for (int i = n + 1; i <= n + 100; i++) {
        if (single_level.search(i)) sl_absent++;
        if (twolevel.search(i)) tl_absent++;
    }
    std::cout << "  Single-level false positives: " << sl_absent << "/100\n";
    std::cout << "  Two-level false positives:    " << tl_absent << "/100\n\n";

    // --- Space scaling ---
    std::cout << "--- Space Scaling ---\n";
    std::cout << std::setw(8) << "n"
              << std::setw(14) << "2level_space"
              << std::setw(12) << "ratio"
              << "\n";
    for (int sz : {100, 500, 1000, 3000, 5000}) {
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

    std::cout << "\nTwo-level hashing: O(n) space, O(1) lookup, simple to implement.\n";
    return 0;
}
