// =====================================================================
// Treap (Tree + Heap)
//
// A treap is a binary search tree where each node also has a randomly
// assigned priority. The structure satisfies:
//   - BST property on keys
//   - Min-heap property on priorities (smaller = higher priority)
//
// Random priorities make the expected height O(log n) regardless
// of insertion order. Insert and delete use rotations to restore
// both invariants.
//
// This example demonstrates insert, search, remove, and verifies
// both the BST and heap properties after each operation.
// =====================================================================

#include "ral/treap.h"
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
#include <cmath>

using namespace ral;

int main() {
    std::cout << "=============================================\n";
    std::cout << "  Treap (BST + Heap)\n";
    std::cout << "=============================================\n\n";

    std::mt19937 rng(42);

    // --- Basic operations demo ---
    Treap treap(rng);
    std::cout << "Inserting keys 1..20:\n";
    for (int i = 1; i <= 20; i++)
        treap.insert(i);

    std::cout << "  Size:   " << treap.size() << "\n";
    std::cout << "  Height: " << treap.height() << "\n";
    std::cout << "  BST OK: " << (treap.verify_bst() ? "YES" : "NO!") << "\n";
    std::cout << "  Heap OK: " << (treap.verify_heap() ? "YES" : "NO!") << "\n\n";

    // In-order traversal shows sorted order
    auto sorted = treap.inorder();
    std::cout << "In-order (first 20): ";
    for (int i = 0; i < (int)sorted.size() && i < 20; i++)
        std::cout << sorted[i] << " ";
    std::cout << "\n\n";

    // Search
    std::cout << "search(10) -> " << (treap.search(10) ? "FOUND" : "not found") << "\n";
    std::cout << "search(99) -> " << (treap.search(99) ? "FOUND" : "not found") << "\n\n";

    // Remove
    treap.remove(10);
    std::cout << "After remove(10):\n";
    std::cout << "  search(10) -> " << (treap.search(10) ? "FOUND" : "not found") << "\n";
    std::cout << "  Size:        " << treap.size() << "\n";
    std::cout << "  BST OK:      " << (treap.verify_bst() ? "YES" : "NO!") << "\n";
    std::cout << "  Heap OK:     " << (treap.verify_heap() ? "YES" : "NO!") << "\n\n";

    // --- Height analysis ---
    std::cout << "--- Height Analysis (10000 trials) ---\n";
    std::cout << std::setw(8) << "n"
              << std::setw(12) << "avg_height"
              << std::setw(12) << "2*lg(n)"
              << std::setw(10) << "ratio"
              << "\n";

    for (int n : {100, 500, 1000, 5000, 10000}) {
        double total_height = 0;
        constexpr int trials = 10000;
        for (int t = 0; t < trials; t++) {
            std::mt19937 trial_rng(t * 12345 + n);
            Treap treap2(trial_rng);
            for (int i = 1; i <= n; i++)
                treap2.insert(i);
            total_height += treap2.height();
        }
        double avg = total_height / trials;
        double theoretical = 2.0 * std::log2(n);
        std::cout << std::setw(8) << n
                  << std::setw(12) << std::fixed << std::setprecision(1) << avg
                  << std::setw(12) << std::fixed << std::setprecision(1) << theoretical
                  << std::setw(10) << std::fixed << std::setprecision(2) << avg / theoretical
                  << "\n";
    }

    std::cout << "\nTheory: expected height = O(lg n), ratio to 2*lg(n) approaches 1.\n";
    return 0;
}
