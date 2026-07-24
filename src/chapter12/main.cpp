#include "dnf_counting.h"
#include "volume.h"
#include <string>

void print_header(const std::string& title) {
    println("\n========================================");
    println("  {}", title);
    println("========================================\n");
}

int main() {
    println("Chapter 11: Approximate Counting");
    println("DNF Counting & Volume of a Convex Body");
    println("Randomized Algorithms -- C++23 Implementations\n");

    print_header("11.1 DNF Counting (Karp-Luby Algorithm)");
    randalgo::demonstrate_dnf_counting();

    print_header("11.2 Volume of a Convex Body");
    randalgo::demonstrate_volume();

    println("========================================");
    println("  All demonstrations complete.");
    println("========================================\n");

    return 0;
}
