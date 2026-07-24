#include "ral.h"
#include <iostream>
#include <random>

int main() {
    std::cout << "========================================================\n";
    std::cout << "        RAL Data Structures Examples\n";
    std::cout << "========================================================\n\n";

    std::mt19937 rng(1337);

    // ----------------------------------------------------------------
    // 1. Treap Demonstration
    // ----------------------------------------------------------------
    std::cout << "--- 1. Treap (BST + Heap) ---\n";
    ral::Treap treap(rng);
    std::cout << "  Inserting: 10, 20, 5, 15, 30, 25\n";
    treap.insert(10);
    treap.insert(20);
    treap.insert(5);
    treap.insert(15);
    treap.insert(30);
    treap.insert(25);

    std::cout << "  Treap size: " << treap.size() << "\n";
    std::cout << "  Treap height: " << treap.height() << "\n";
    std::cout << "  Searching for 15: " << (treap.search(15) ? "Found" : "Not Found") << "\n";
    std::cout << "  Searching for 100: " << (treap.search(100) ? "Found" : "Not Found") << "\n";

    std::cout << "  Removing 20 and 5\n";
    treap.remove(20);
    treap.remove(5);
    std::cout << "  Treap size: " << treap.size() << "\n";
    std::cout << "  Searching for 20: " << (treap.search(20) ? "Found" : "Not Found") << "\n\n";

    // ----------------------------------------------------------------
    // 2. Skip List Demonstration
    // ----------------------------------------------------------------
    std::cout << "--- 2. Skip List ---\n";
    ral::SkipList skip_list(rng);
    std::cout << "  Inserting: 50, 30, 70, 20, 40, 60, 80\n";
    skip_list.insert(50);
    skip_list.insert(30);
    skip_list.insert(70);
    skip_list.insert(20);
    skip_list.insert(40);
    skip_list.insert(60);
    skip_list.insert(80);

    std::cout << "  Skip list size: " << skip_list.size() << "\n";
    std::cout << "  Max level reached: " << skip_list.max_level() << "\n";
    std::cout << "  Searching for 40: " << (skip_list.search(40) ? "Found" : "Not Found") << "\n";

    std::cout << "  Removing 50\n";
    skip_list.remove(50);
    std::cout << "  Skip list size: " << skip_list.size() << "\n";
    std::cout << "  Searching for 50: " << (skip_list.search(50) ? "Found" : "Not Found") << "\n\n";

    // ----------------------------------------------------------------
    // 3. Universal Hash Table Demonstration (Chaining)
    // ----------------------------------------------------------------
    std::cout << "--- 3. Universal Hash Table (Chained) ---\n";
    // We instantiate with table size 10
    ral::ChainedHashTable chained_table(10, rng);
    std::cout << "  Inserting keys: 12, 22, 32, 5, 15\n";
    chained_table.insert(12);
    chained_table.insert(22);
    chained_table.insert(32);
    chained_table.insert(5);
    chained_table.insert(15);

    std::cout << "  Hash table size: " << chained_table.size() << "\n";
    std::cout << "  Load factor: " << chained_table.load_factor() << "\n";
    std::cout << "  Average chain length: " << chained_table.avg_chain_length() << "\n";
    std::cout << "  Max chain length: " << chained_table.max_chain_length() << "\n";
    std::cout << "  Searching for 22: " << (chained_table.search(22) ? "Found" : "Not Found") << "\n";

    std::cout << "  Removing 22\n";
    chained_table.remove(22);
    std::cout << "  Searching for 22: " << (chained_table.search(22) ? "Found" : "Not Found") << "\n\n";

    // ----------------------------------------------------------------
    // 4. Perfect Hash Table Demonstration (FKS 2-Level Hashing)
    // ----------------------------------------------------------------
    std::cout << "--- 4. Perfect Hash Table (FKS Hashing) ---\n";
    ral::PerfectHashTable perfect_table(10, rng);
    std::cout << "  Inserting keys: 1, 4, 9, 16, 25, 36, 49\n";
    perfect_table.insert(1);
    perfect_table.insert(4);
    perfect_table.insert(9);
    perfect_table.insert(16);
    perfect_table.insert(25);
    perfect_table.insert(36);
    perfect_table.insert(49);

    std::cout << "  Perfect hash table size: " << perfect_table.size() << "\n";
    std::cout << "  Searching for 25: " << (perfect_table.search(25) ? "Found" : "Not Found") << "\n";
    std::cout << "  Searching for 100: " << (perfect_table.search(100) ? "Found" : "Not Found") << "\n\n";

    std::cout << "========================================================\n\n";
    return 0;
}
