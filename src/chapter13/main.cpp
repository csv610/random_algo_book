#include "pram_simulation.h"
#include "mis.h"
#include "matchings.h"
#include "../compat_print.h"

int main() {
    println("+==============================================================+");
    println("|       Chapter 12: Parallel and Distributed Algorithms       |");
    println("+==============================================================+");

    randalgo::demonstrate_pram();
    randalgo::demonstrate_mis();
    randalgo::demonstrate_matchings();

    println("\nDone.");
    return 0;
}
