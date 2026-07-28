#include "ral/conditional_expectation.h"
#include <iostream>

int main() {
    using namespace ral;

    // Example CNF: (x1 OR x2) AND (NOT x1 OR x3) AND (x2 OR NOT x3)
    std::vector<CNFClause> clauses = {
        {{1, 2}},       // x1 OR x2
        {{-1, 3}},      // NOT x1 OR x3
        {{2, -3}}       // x2 OR NOT x3
    };

    println("=== Derandomized MAX-SAT ===");
    println("Formula: (x1 OR x2) AND (NOT x1 OR x3) AND (x2 OR NOT x3)");

    auto derand = derandomized_max_sat(clauses, 3);
    println("Derandomized: {}/{} clauses satisfied", derand.clauses_satisfied, derand.total_clauses);
    print("  Assignment: ");
    for (bool b : derand.assignment) print("{} ", b ? 1 : 0);
    println("");

    auto random = random_max_sat(clauses, 3, 10000, 42);
    println("Best random (10000 trials): {}/{} clauses", random.clauses_satisfied, random.total_clauses);

    // Larger example
    println("\n=== Larger MAX-SAT (5 vars, 20 clauses) ===");
    std::vector<CNFClause> large_clauses;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> var_dist(1, 5);
    std::uniform_int_distribution<int> sign_dist(0, 1);

    for (int i = 0; i < 20; ++i) {
        CNFClause c;
        int len = 2 + (i % 3);
        for (int j = 0; j < len; ++j) {
            int v = var_dist(rng);
            c.literals.push_back((sign_dist(rng) == 0) ? v : -v);
        }
        large_clauses.push_back(c);
    }

    auto large_derand = derandomized_max_sat(large_clauses, 5);
    auto large_random = random_max_sat(large_clauses, 5, 10000, 42);
    println("Derandomized: {}/20", large_derand.clauses_satisfied);
    println("Best random:  {}/20", large_random.clauses_satisfied);

    // Discrepancy
    println("\n=== Derandomized Discrepancy ===");
    std::vector<std::vector<int>> sets = {{0,1,2}, {1,2,3}, {0,3}};
    auto disc = derandomized_discrepancy(sets, 4);
    println("Max discrepancy: {}", disc.max_discrepancy);
    print("  Coloring: ");
    for (int c : disc.coloring) print("{} ", c);
    println("");

    return 0;
}
