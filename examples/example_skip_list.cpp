// =====================================================================
// Skip List
//
// A probabilistic data structure that maintains a sorted sequence
// with O(log n) expected time for search, insert, and delete.
//
// Each element is promoted to level i with probability 1/2^i.
// Level 0 contains all elements; higher levels act as express lanes.
// Search starts at the top level and skips forward, dropping down
// when the next element overshoots the target.
//
// Expected space: O(n), expected height: O(log n).
// =====================================================================

#include "ral/skip_list.h"
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Skip List\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    // --- Insert / search / remove demo ---
    SkipList sl(rng);
    std::cout << "Inserting keys 1..20:\n";
    for (int i = 1; i <= 20; i++)
        sl.insert(i);

    std::cout << "  Size:      " << sl.size() << "\n";
    std::cout << "  Max level: " << sl.max_level() << "\n\n";

    sl.print();

    std::cout << "\nsearch(10) -> " << (sl.search(10) ? "FOUND" : "not found") << "\n";
    std::cout << "search(99) -> " << (sl.search(99) ? "FOUND" : "not found") << "\n\n";

    sl.remove(10);
    std::cout << "After remove(10):\n";
    std::cout << "  search(10) -> " << (sl.search(10) ? "FOUND" : "not found") << "\n";
    std::cout << "  Size:       " << sl.size() << "\n\n";

    // --- Height analysis ---
    std::cout << "--- Height Analysis (1000 trials) ---\n";
    std::cout << std::setw(8) << "n"
              << std::setw(12) << "avg_height"
              << std::setw(12) << "lg(n)"
              << "\n";

    for (int n : {100, 500, 1000, 5000, 10000}) {
        double total_height = 0;
        constexpr int trials = 1000;
        for (int t = 0; t < trials; t++) {
            std::mt19937 trial_rng(t * 99991 + n);
            SkipList sl2(trial_rng);
            for (int i = 1; i <= n; i++)
                sl2.insert(i);
            total_height += sl2.max_level();
        }
        double avg = total_height / trials;
        double theoretical = std::log2(n);
        std::cout << std::setw(8) << n
                  << std::setw(12) << std::fixed << std::setprecision(1) << avg
                  << std::setw(12) << std::fixed << std::setprecision(1) << theoretical
                  << "\n";
    }

    std::cout << "\nTheory: expected height = O(lg n), each level halves the list.\n";
    return 0;
}
