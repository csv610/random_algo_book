#include "convex_hull.h"
#include "delaunay.h"
#include "linear_programming.h"
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
    std::cout << "Chapter 9: Geometric Algorithms and Linear Programming\n";
    std::cout << "Randomized Algorithms - C++ Implementations\n";

    print_header("9.2 Convex Hull (Randomized Incremental)");
    chapter9::demonstrate_convex_hull();

    print_header("9.5 Delaunay Triangulation (Incremental with Flips)");
    chapter9::demonstrate_delaunay();

    print_header("9.10 Linear Programming (Seidel's Algorithm)");
    chapter9::demonstrate_lp();

    std::cout << "\n========================================\n";
    std::cout << "  All demonstrations complete.\n";
    std::cout << "========================================\n";

    return 0;
}
