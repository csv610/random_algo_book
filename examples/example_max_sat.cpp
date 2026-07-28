#include "ral/max_sat.h"
#include <iostream>

int main() {
    using namespace ral;

    // Create random MAX-SAT instance
    println("=== MAX-SAT Algorithms ===");
    auto inst = random_max_sat_instance(5, 15, 3, 42);
    println("Instance: {} variables, {} clauses", inst.num_vars, inst.clauses.size());

    // Random assignment
    auto random = random_assignment(inst, 10000, 42);
    println("Random (best of 10000): {:.1f} weight satisfied", random.weight_satisfied);

    // Greedy
    auto greedy = greedy_max_sat(inst, 100, 42);
    println("Greedy:                {:.1f} weight satisfied", greedy.weight_satisfied);

    // Randomized rounding
    auto rounding = randomized_rounding_max_sat(inst, 42);
    println("Randomized rounding:  {:.1f} weight satisfied", rounding.weight_satisfied);

    // Theoretical lower bound
    double expected = 0.0;
    for (const auto& c : inst.clauses) {
        double prob = 1.0 - std::pow(0.5, c.literals.size());
        expected += c.weight * prob;
    }
    println("Random expected bound: {:.1f}", expected);

    // Larger instance
    println("\n=== Larger Instance (10 vars, 50 clauses) ===");
    auto inst2 = random_max_sat_instance(10, 50, 4, 123);
    auto r2 = random_assignment(inst2, 10000, 123);
    auto g2 = greedy_max_sat(inst2, 200, 123);
    auto rr2 = randomized_rounding_max_sat(inst2, 123);
    println("Random:    {:.1f}", r2.weight_satisfied);
    println("Greedy:    {:.1f}", g2.weight_satisfied);
    println("Rounding:  {:.1f}", rr2.weight_satisfied);

    return 0;
}
