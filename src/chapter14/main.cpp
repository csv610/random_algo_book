#include <iostream>
#include <string>
#include "paging.h"
#include "k_server.h"
#include "adversary.h"

namespace {

void print_header(const std::string& title) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "========================================\n\n";
}

} // anonymous namespace

int main() {
    println("Chapter 13: Online Algorithms");
    println("Randomized Algorithms - C++ Implementations\n");

    print_header("13.1 Paging Algorithms");
    randalgo::demonstrate_paging();

    print_header("13.2 The k-Server Problem");
    randalgo::demonstrate_k_server();

    print_header("13.3 Adversary Models");
    randalgo::demonstrate_adversary();

    std::cout << "\n========================================\n";
    std::cout << "  All demonstrations complete.\n";
    std::cout << "========================================\n";

    return 0;
}
