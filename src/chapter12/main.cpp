#include "pram_simulation.h"
#include "mis.h"
#include "matchings.h"
#include <print>

int main() {
    std::println("+==============================================================+");
    std::println("|       Chapter 12: Parallel and Distributed Algorithms       |");
    std::println("+==============================================================+");

    randalgo::demonstrate_pram();
    randalgo::demonstrate_mis();
    randalgo::demonstrate_matchings();

    std::println("\nDone.");
    return 0;
}
