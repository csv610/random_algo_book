#include <print>
#include <string>

#include "number_theory.h"
#include "crypto.h"
#include "polynomial.h"

static void print_header(const std::string& title) {
    std::println("\n========================================");
    std::println("  {}", title);
    std::println("========================================\n");
}

int main() {
    print_header("Chapter 14: Number Theory and Algebra");

    randalgo::demonstrate_number_theory();

    print_header("Chapter 14: Cryptographic Algorithms");

    randalgo::demonstrate_crypto();

    print_header("Chapter 14: Polynomial Operations");

    randalgo::demonstrate_polynomial();

    std::println("\n========================================");
    std::println("  Chapter 14 Complete");
    std::println("========================================\n");

    return 0;
}
