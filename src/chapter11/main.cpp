#include "apsp.h"
#include "min_cut.h"
#include "mst.h"
#include <iostream>
#include <string>

void print_header(const std::string& title) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "========================================\n\n";
}

int main() {
    std::cout << "Chapter 10: Graph Algorithms\n";
    std::cout << "Randomized Algorithms - C++ Implementations\n";

    print_header("10.1 All-Pairs Shortest Paths");
    randalgo::demonstrate_apsp();

    print_header("10.2 The Min-Cut Problem (Karger & Karger-Stein)");
    randalgo::demonstrate_min_cut();

    print_header("10.3 Minimum Spanning Trees (Karger-Klein-Tarjan)");
    randalgo::demonstrate_mst();

    std::cout << "\n========================================\n";
    std::cout << "  All demonstrations complete.\n";
    std::cout << "========================================\n";

    return 0;
}
