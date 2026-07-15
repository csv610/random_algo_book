#include "treap.h"
#include "skip_list.h"
#include "hash_table.h"
#include <iostream>
#include <string>
#include <chrono>

void print_header(const std::string& title) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "========================================\n\n";
}

void print_section(const std::string& title) {
    std::cout << "\n--- " << title << " ---\n\n";
}

int main() {
    std::cout << "Chapter 8: Data Structures\n";
    std::cout << "Randomized Algorithms - C++ Implementations\n";

    print_header("8.2 Random Treaps");
    chapter8::demonstrate_treap();

    print_header("8.3 Skip Lists");
    chapter8::demonstrate_skip_list();

    print_header("8.4 - 8.5 Hash Tables");
    chapter8::demonstrate_hashing();

    std::cout << "\n========================================\n";
    std::cout << "  All demonstrations complete.\n";
    std::cout << "========================================\n";

    return 0;
}
