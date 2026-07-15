#pragma once

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <string>
#include <functional>

namespace chapter5 {

struct LLLResult {
    bool condition_satisfied;
    double max_ratio; // max over i of Pr[A_i] / (x_i * prod(1-x_j))
};

// Verify the asymmetric LLL condition: for each i,
// Pr[A_i] <= x_i * prod_{j in deps[i]} (1 - x_j).
// Returns whether the condition holds, and the maximum ratio.
LLLResult local_lemma_check(const std::vector<double>& probs,
                            const std::vector<std::vector<int>>& deps,
                            int max_iter) {
    int m = static_cast<int>(probs.size());
    std::vector<double> x(m, 1.0 / 2.0);

    // Iterate to find feasible x values (simple fixed-point approach)
    for (int iter = 0; iter < max_iter; ++iter) {
        for (int i = 0; i < m; ++i) {
            double prod = 1.0;
            for (int j : deps[i])
                prod *= (1.0 - x[j]);
            if (prod > 1e-15) {
                x[i] = std::min(probs[i] / prod, 0.9999);
            } else {
                x[i] = 0.9999;
            }
        }
    }

    // Check the condition
    double max_ratio = 0.0;
    bool satisfied = true;
    for (int i = 0; i < m; ++i) {
        double prod = 1.0;
        for (int j : deps[i])
            prod *= (1.0 - x[j]);
        double rhs = x[i] * prod;
        if (rhs < 1e-15) {
            satisfied = false;
            max_ratio = 1e9;
        } else {
            double ratio = probs[i] / rhs;
            max_ratio = std::max(max_ratio, ratio);
            if (probs[i] > rhs + 1e-12)
                satisfied = false;
        }
    }

    LLLResult res;
    res.condition_satisfied = satisfied;
    res.max_ratio = max_ratio;
    return res;
}

// Check symmetric LLL condition: ep(d+1) <= 1.
bool symmetric_lll_check(double p, int d) {
    return std::exp(1.0) * p * (d + 1) <= 1.0;
}

// Apply LLL to 2-SAT: given a formula with m clauses over n variables,
// check whether the LLL condition guarantees satisfiability.
void demonstrate_2sat() {
    std::cout << "Lovasz Local Lemma: 2-SAT Application\n";
    std::cout << std::string(40, '-') << "\n\n";

    std::cout << "For 2-SAT with m clauses and clause-variable density d:\n";
    std::cout << "  Pr[clause unsatisfied] = 1/4\n";
    std::cout << "  Each clause shares vars with at most 2(l_j - 1) other clauses\n";
    std::cout << "  Symmetric LLL: e*(1/4)*(d+1) <= 1  =>  d <= 4/e - 1 ~ 0.47\n\n";

    std::cout << std::setw(10) << "n_vars"
              << std::setw(10) << "n_clauses"
              << std::setw(8) << "Density"
              << std::setw(12) << "Max degree"
              << std::setw(12) << "LLL holds?"
              << "\n";
    std::cout << std::string(52, '-') << "\n";

    struct TestCase { int n; int m; };
    std::vector<TestCase> tests = {
        {10, 5}, {10, 8}, {10, 10},
        {20, 10}, {20, 15}, {20, 20},
        {50, 25}, {50, 40}, {50, 50},
        {100, 50}, {100, 80}, {100, 100}
    };

    for (const auto& tc : tests) {
        // Build dependency graph: clauses that share a variable are dependent
        // For random 2-SAT, estimate max dependency degree
        int max_dep = std::min(2 * 2 * (tc.m / tc.n) * tc.n, tc.m);
        // Rough estimate: each variable appears in ~2m/n clauses, each clause
        // has 2 vars, so each clause shares a variable with at most 2*(2m/n - 1) others
        double density = static_cast<double>(tc.m) / tc.n;
        max_dep = std::min(static_cast<int>(4 * density), tc.m);

        bool lll = symmetric_lll_check(0.25, max_dep);

        std::cout << std::setw(10) << tc.n
                  << std::setw(10) << tc.m
                  << std::setw(8) << std::setprecision(2) << density
                  << std::setw(12) << max_dep
                  << std::setw(12) << (lll ? "Yes" : "No")
                  << "\n";
    }

    std::cout << "\nThe LLL gives a sufficient (not necessary) condition.\n";
    std::cout << "Sparse 2-SAT formulas (low density) are guaranteed satisfiable.\n";
}

// Apply LLL to hypergraph coloring: 2-color a k-uniform hypergraph
// with no monochromatic edge.
void demonstrate_hypergraph() {
    std::cout << "\nLovasz Local Lemma: Hypergraph 2-Coloring\n";
    std::cout << std::string(44, '-') << "\n\n";

    std::cout << "For a k-uniform hypergraph with m edges and max degree Delta:\n";
    std::cout << "  Pr[edge monochromatic] = 2^{1-k}\n";
    std::cout << "  Dependency degree: at most k*Delta\n";
    std::cout << "  Symmetric LLL: e * 2^{1-k} * (k*Delta + 1) <= 1\n\n";

    std::cout << std::setw(6) << "k"
              << std::setw(10) << "Delta_max"
              << std::setw(15) << "Max m for LLL"
              << std::setw(12) << "Pr[mono]"
              << "\n";
    std::cout << std::string(43, '-') << "\n";

    for (int k = 3; k <= 8; ++k) {
        double p_mono = std::pow(2.0, 1.0 - k);
        // From LLL: e * p_mono * (k*Delta + 1) <= 1
        // => k*Delta <= 1/(e*p_mono) - 1
        double max_kd = 1.0 / (std::exp(1.0) * p_mono) - 1.0;
        int max_delta = static_cast<int>(max_kd / k);
        int max_m = max_delta; // each vertex in at most Delta edges, m edges cover <= k*m vertices

        std::cout << std::setw(6) << k
                  << std::setw(10) << max_delta
                  << std::setw(15) << max_m
                  << std::setw(12) << std::setprecision(6) << p_mono
                  << "\n";
    }

    std::cout << "\nFor k=3 (3-uniform): LLL guarantees 2-coloring for small hypergraphs.\n";
    std::cout << "As k grows, the guarantee becomes stronger (lower Pr[mono]).\n";
}

void demonstrate_lovasz() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "The Lovasz Local Lemma\n";
    std::cout << std::string(40, '=') << "\n\n";

    demonstrate_2sat();
    demonstrate_hypergraph();

    // Demonstrate LLL verification on a small example
    std::cout << "\n\nLLL Verification on a Small Example\n";
    std::cout << std::string(40, '-') << "\n\n";

    // 5 events, each with prob 0.1, dependency degree 2
    // Condition: e * 0.1 * 3 = 0.815 <= 1 -- holds
    int m = 5;
    std::vector<double> probs(m, 0.1);
    std::vector<std::vector<int>> deps(m);
    // Chain dependency: 0-1, 1-2, 2-3, 3-4
    deps[0] = {1};
    deps[1] = {0, 2};
    deps[2] = {1, 3};
    deps[3] = {2, 4};
    deps[4] = {3};

    auto result = local_lemma_check(probs, deps, 100);

    std::cout << "Events: A_0, ..., A_4 with Pr[A_i] = 0.1\n";
    std::cout << "Dependency graph: chain 0-1-2-3-4\n";
    std::cout << "Max dependency degree: 2\n";
    std::cout << "Symmetric LLL condition: e * 0.1 * 3 = "
              << std::exp(1.0) * 0.1 * 3.0 << " <= 1: "
              << (symmetric_lll_check(0.1, 2) ? "Yes" : "No") << "\n";
    std::cout << "Asymmetric LLL condition satisfied: "
              << (result.condition_satisfied ? "Yes" : "No") << "\n";
    std::cout << "Max ratio (Pr[A_i] / RHS): " << result.max_ratio << "\n";

    std::cout << "\nKey takeaways:\n";
    std::cout << "  - LLL is powerful when events have limited dependencies\n";
    std::cout << "  - Symmetric form: ep(d+1) <= 1 is easy to verify\n";
    std::cout << "  - Asymmetric form: choose x_i to maximize the product term\n";
    std::cout << "  - Applications: satisfiability, coloring, embedding problems\n";
}

} // namespace chapter5
