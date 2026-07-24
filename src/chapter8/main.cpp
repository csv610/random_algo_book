#include <iostream>
#include <iomanip>
#include <string>
#include "freivalds.h"
#include "polynomial.h"
#include "rabin_karp.h"

void print_header(const std::string& title) {
    std::cout << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

void print_section(const std::string& title) {
    std::cout << "\n" << std::string(50, '-') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(50, '-') << "\n\n";
}

int main() {
    print_header("Chapter 7: Algebraic Techniques");
    std::cout << "Randomized Algorithms - Implementation Demos\n\n";

    print_section("7.1 Freivalds' Matrix Multiplication Verification");
    chapter8::demonstrate_freivalds();

    print_section("7.2 Polynomial Identity Testing (Schwartz-Zippel)");
    chapter8::demonstrate_polynomial();

    print_section("7.6 Rabin-Karp Pattern Matching");
    chapter8::demonstrate_rabin_karp();

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  All demos completed.\n";
    std::cout << std::string(60, '=') << "\n";

    return 0;
}
