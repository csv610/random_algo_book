#include "../compat_print.h"
#include <string>

#include "number_theory.h"
#include "crypto.h"
#include "polynomial.h"

static void print_header(const std::string& title) {
    println("\n========================================");
    println("  {}", title);
    println("========================================\n");
}

int main() {
    print_header("Chapter 14: Number Theory and Algebra");

    randalgo::demonstrate_number_theory();

    print_header("Chapter 14: Cryptographic Algorithms");

    randalgo::demonstrate_crypto();

    print_header("Chapter 14: Polynomial Operations");

    randalgo::demonstrate_polynomial();

    println("\n========================================");
    println("  Chapter 14 Complete");
    println("========================================\n");

    return 0;
}
