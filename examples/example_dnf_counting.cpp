#include "ral/dnf_counting.h"
#include <iostream>

int main() {
    using namespace ral;

    // DNF: (x1 AND x2) OR (NOT x1 AND x3) OR (x2 AND NOT x3)
    // 3 variables, 3 clauses
    std::vector<DNFClause> clauses = {
        {{1, 2}},       // x1 AND x2
        {{-1, 3}},      // NOT x1 AND x3
        {{2, -3}}       // x2 AND NOT x3
    };

    println("=== Karp-Luby DNF Counting ===");
    println("Formula: (x1 AND x2) OR (NOT x1 AND x3) OR (x2 AND NOT x3)");

    // Exact count
    double exact = exact_dnf_count(3, clauses);
    println("Exact count: {:.0f} out of 8 assignments", exact);

    // Karp-Luby approximation
    for (double eps : {0.5, 0.2, 0.1, 0.05}) {
        auto result = karp_luby_dnf(3, clauses, eps, 42);
        println("  epsilon={:.2f}: estimate={:.2f}, exact={:.0f}, rel_error={:.4f}, within_tol={}",
                eps, result.estimate, result.exact_count,
                result.relative_error, result.within_tolerance ? "YES" : "NO");
    }

    // Larger DNF
    println("\n=== Larger DNF (6 vars, 10 clauses) ===");
    std::vector<DNFClause> large_clauses;
    for (int i = 0; i < 10; ++i) {
        DNFClause c;
        int len = 2 + (i % 3);
        for (int j = 0; j < len; ++j) {
            int var = (i + j) % 6 + 1;
            c.literals.push_back((j % 2 == 0) ? var : -var);
        }
        large_clauses.push_back(c);
    }
    auto large_exact = exact_dnf_count(6, large_clauses);
    auto large_est = karp_luby_dnf(6, large_clauses, 0.1, 42);
    println("Exact: {:.0f}, Estimate: {:.2f}, Error: {:.4f}",
            large_exact, large_est.estimate, large_est.relative_error);

    return 0;
}
