#include "dnf_counting.h"
#include "volume.h"
#include <string>

void print_header(const std::string& title) {
    std::println("\n========================================");
    std::println("  {}", title);
    std::println("========================================\n");
}

int main() {
    std::println("Chapter 11: Approximate Counting");
    std::println("DNF Counting & Volume of a Convex Body");
    std::println("Randomized Algorithms -- C++23 Implementations\n");

    print_header("11.1 DNF Counting (Karp-Luby Algorithm)");
    randalgo::demonstrate_dnf_counting();

    print_header("11.2 Volume of a Convex Body");
    randalgo::demonstrate_volume();

    std::println("========================================");
    std::println("  All demonstrations complete.");
    std::println("========================================\n");

    return 0;
}
