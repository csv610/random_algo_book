#pragma once

#include <vector>
#include <random>
#include "../compat_print.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cassert>

namespace randalgo {

// ---------------------------------------------------------------------------
// Data structures for DNF formulas
// ---------------------------------------------------------------------------

struct Literal {
    int variable;      // 0-based variable index
    bool is_negated;   // true means ~x_variable

    Literal(int v, bool neg = false) : variable(v), is_negated(neg) {}
};

struct Clause {
    std::vector<Literal> literals;

    Clause() = default;
    Clause(std::vector<Literal> lits) : literals(std::move(lits)) {}
};

struct DNF {
    std::vector<Clause> clauses;

    DNF() = default;
    DNF(std::vector<Clause> cs) : clauses(std::move(cs)) {}
};

// ---------------------------------------------------------------------------
// Random truth assignment: each variable independently true/false
// ---------------------------------------------------------------------------

std::vector<bool> truth_assignment(int n, std::mt19937& rng) {
    std::vector<bool> assignment(n);
    std::uniform_int_distribution<int> coin(0, 1);
    for (int i = 0; i < n; ++i)
        assignment[i] = static_cast<bool>(coin(rng));
    return assignment;
}

// ---------------------------------------------------------------------------
// Evaluate DNF: a DNF is satisfied if ANY clause is satisfied.
// A clause is satisfied if ALL its literals evaluate to true.
// ---------------------------------------------------------------------------

bool evaluate_clause(const Clause& clause, const std::vector<bool>& assignment) {
    return std::ranges::all_of(clause.literals, [&](const Literal& lit) {
        bool val = assignment[lit.variable];
        return lit.is_negated ? !val : val;
    });
}

bool evaluate_dnf(const DNF& dnf, const std::vector<bool>& assignment) {
    return std::ranges::any_of(dnf.clauses, [&](const Clause& c) {
        return evaluate_clause(c, assignment);
    });
}

// ---------------------------------------------------------------------------
// Brute-force: enumerate all 2^n assignments
// ---------------------------------------------------------------------------

int count_satisfying_assignments_exact(const DNF& dnf, int n) {
    int count = 0;
    const int total = 1 << n;
    for (int mask = 0; mask < total; ++mask) {
        std::vector<bool> assignment(n);
        for (int i = 0; i < n; ++i)
            assignment[i] = static_cast<bool>((mask >> i) & 1);
        if (evaluate_dnf(dnf, assignment))
            ++count;
    }
    return count;
}

// ---------------------------------------------------------------------------
// Karp-Luby approximate counting for DNF formulas
//
// Given DNF = C_1 | C_2 | ... | C_k, let S_i = {assignments satisfying C_i}.
// We want m = |S_1 | S_2 | ... | S_k|.
//
// Key insight: |S_i| = 2^(n - l_i) where l_i = distinct variables in C_i.
// Let W_i = |S_i| and W = Sum_{i} W_i.
//
// Algorithm (importance sampling):
//   1. Pick clause i with probability  W_i / W.
//   2. Sample x uniformly from S_i.
//   3. Let c(x) = number of clauses satisfied by x.
//   4. Estimate = W / c(x).
//
// Proof:  Pr[outputting x] = Sum_{i: x in S_i} (W_i/W).(1/|S_i|) = c(x)/W.
//         E[W/c(x)] = Sigma_x Pr[x].(W/c(x)) = Sigma_x (c(x)/W).(W/c(x)) = m.
// ---------------------------------------------------------------------------

double karp_luby_dnf(const DNF& dnf, int n, int trials, std::mt19937& rng) {
    const int k = static_cast<int>(dnf.clauses.size());
    if (k == 0) return 0.0;

    // Compute |S_i| for each clause and the total W
    std::vector<double> weights(k);
    for (int i = 0; i < k; ++i) {
        std::vector<bool> seen(n, false);
        for (const auto& lit : dnf.clauses[i].literals)
            seen[lit.variable] = true;
        int distinct = static_cast<int>(std::ranges::count(seen, true));
        weights[i] = std::ldexp(1.0, n - distinct);  // 2^(n - distinct)
    }
    double W = std::accumulate(weights.begin(), weights.end(), 0.0);

    // Cumulative weights for sampling clause i with probability W_i / W
    std::vector<double> cum(k);
    std::partial_sum(weights.begin(), weights.end(), cum.begin());

    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    std::uniform_int_distribution<int> coin(0, 1);

    double total_estimate = 0.0;
    for (int t = 0; t < trials; ++t) {
        // Step 1: pick clause i with probability W_i / W
        double r = uniform(rng) * W;
        int idx = static_cast<int>(
            std::ranges::lower_bound(cum, r) - cum.begin());
        idx = std::clamp(idx, 0, k - 1);

        // Step 2: sample x uniformly from S_idx
        //   Set literals in the clause to their satisfying values;
        //   remaining variables are assigned randomly.
        std::vector<bool> in_clause(n, false);
        for (const auto& lit : dnf.clauses[idx].literals)
            in_clause[lit.variable] = true;

        std::vector<bool> assignment(n);
        for (int v = 0; v < n; ++v) {
            if (in_clause[v]) {
                // Find the literal for variable v in clause idx
                bool negate = false;
                for (const auto& lit : dnf.clauses[idx].literals) {
                    if (lit.variable == v) { negate = lit.is_negated; break; }
                }
                assignment[v] = !negate;  // satisfying value
            } else {
                assignment[v] = static_cast<bool>(coin(rng));
            }
        }

        // Step 3: count c(x) = number of clauses satisfied by x
        int c_x = 0;
        for (int j = 0; j < k; ++j) {
            if (evaluate_clause(dnf.clauses[j], assignment))
                ++c_x;
        }

        // Step 4: unbiased estimate = W / c(x)
        total_estimate += W / static_cast<double>(c_x);
    }

    return total_estimate / static_cast<double>(trials);
}

// ---------------------------------------------------------------------------
// Count unique satisfying assignments via sampling (for comparison)
// ---------------------------------------------------------------------------

double naive_random_sampling_count(const DNF& dnf, int n, int trials,
                                   std::mt19937& rng) {
    int hits = 0;
    for (int t = 0; t < trials; ++t) {
        auto assignment = truth_assignment(n, rng);
        if (evaluate_dnf(dnf, assignment))
            ++hits;
    }
    double p = static_cast<double>(hits) / static_cast<double>(trials);
    return p * std::ldexp(1.0, n);  // p * 2^n
}

// ---------------------------------------------------------------------------
// Generate a random DNF formula
// ---------------------------------------------------------------------------

DNF random_dnf(int n, int num_clauses, int clause_size, std::mt19937& rng) {
    std::uniform_int_distribution<int> var_dist(0, n - 1);
    std::uniform_int_distribution<int> neg_dist(0, 1);

    DNF dnf;
    for (int c = 0; c < num_clauses; ++c) {
        std::vector<Literal> lits;
        // Pick clause_size distinct variables
        std::vector<int> vars(n);
        std::iota(vars.begin(), vars.end(), 0);
        std::ranges::shuffle(vars, rng);
        int sz = std::min(clause_size, n);
        for (int i = 0; i < sz; ++i) {
            bool neg = static_cast<bool>(neg_dist(rng));
            lits.emplace_back(vars[i], neg);
        }
        dnf.clauses.emplace_back(std::move(lits));
    }
    return dnf;
}

// ---------------------------------------------------------------------------
// Print helper
// ---------------------------------------------------------------------------

void print_dnf(const DNF& dnf) {
    bool first_clause = true;
    for (const auto& clause : dnf.clauses) {
        if (!first_clause) print(" | ");
        first_clause = false;
        if (clause.literals.size() > 1) print("(");
        bool first_lit = true;
        for (const auto& lit : clause.literals) {
            if (!first_lit) print(" & ");
            first_lit = false;
            if (lit.is_negated) print("~");
            print("x{}", lit.variable + 1);
        }
        if (clause.literals.size() > 1) print(")");
    }
    println();
}

// ---------------------------------------------------------------------------
// Demonstration
// ---------------------------------------------------------------------------

void demonstrate_dnf_counting() {
    std::mt19937 rng(
        static_cast<unsigned>(std::chrono::steady_clock::now()
                                  .time_since_epoch()
                                  .count()));

    // ---------------------------------------------------------------
    // Part 1: Small DNF formula -- exact enumeration
    // ---------------------------------------------------------------
    println("=== Part 1: Small DNF Formula -- Exact Enumeration ===\n");

    // Formula: (x1 & x2) | (~x1 & x3)
    // With 3 variables, satisfying assignments are:
    //   x1=T,x2=T,x3=?  ->  (T,T,T) and (T,T,F)
    //   x1=F,x3=T,x2=?  ->  (F,T,T) and (F,F,T)
    // That's 4 assignments out of 8.
    DNF small_dnf({
        Clause({Literal(0, false), Literal(1, false)}),    // x1 & x2
        Clause({Literal(0, true),  Literal(2, false)})     // ~x1 & x3
    });

    int n_small = 3;
    print("Formula (n={}): ", n_small);
    print_dnf(small_dnf);

    int exact = count_satisfying_assignments_exact(small_dnf, n_small);
    println("Exact satisfying assignments: {} / {}", exact, 1 << n_small);
    println("Fraction: {:.4f}\n", static_cast<double>(exact) / (1 << n_small));

    // Print all satisfying assignments
    println("Satisfying assignments:");
    for (int mask = 0; mask < (1 << n_small); ++mask) {
        std::vector<bool> a(n_small);
        for (int i = 0; i < n_small; ++i)
            a[i] = static_cast<bool>((mask >> i) & 1);
        if (evaluate_dnf(small_dnf, a)) {
            print("  (");
            for (int i = 0; i < n_small; ++i)
                print("{}x{}", a[i] ? "" : "~", i + 1);
            println(")");
        }
    }

    // ---------------------------------------------------------------
    // Part 2: Karp-Luby vs Exact on the small formula
    // ---------------------------------------------------------------
    println("\n=== Part 2: Karp-Luby Estimator on Small Formula ===\n");

    print("Formula: ");
    print_dnf(small_dnf);
    println("Exact count: {}", exact);
    println();

    for (int trials : {100, 500, 1000, 5000, 10000}) {
        double est = karp_luby_dnf(small_dnf, n_small, trials, rng);
        double err = std::abs(est - exact);
        println("  trials = {:6d}  ->  estimate = {:8.3f}  |error| = {:.3f}",
                     trials, est, err);
    }

    // ---------------------------------------------------------------
    // Part 3: Naive random sampling vs Karp-Luby
    // ---------------------------------------------------------------
    println("\n=== Part 3: Naive Random Sampling vs Karp-Luby ===\n");

    print("Formula: ");
    print_dnf(small_dnf);
    println("Exact count: {}", exact);
    println();

    for (int trials : {1000, 5000, 10000}) {
        double naive_est = naive_random_sampling_count(small_dnf, n_small,
                                                       trials, rng);
        double kl_est = karp_luby_dnf(small_dnf, n_small, trials, rng);
        println("  {:6d} trials:  naive = {:8.3f}  Karp-Luby = {:8.3f}",
                     trials, naive_est, kl_est);
    }

    // ---------------------------------------------------------------
    // Part 4: Larger random DNF -- convergence study
    // ---------------------------------------------------------------
    println("\n=== Part 4: Larger Random DNF -- Convergence Study ===\n");

    int n_large = 10;
    int num_clauses = 50;
    int clause_size = 3;
    DNF large_dnf = random_dnf(n_large, num_clauses, clause_size, rng);

    print("Random DNF (n={}, clauses={}, clause_size={}):\n",
               n_large, num_clauses, clause_size);
    // Print first 5 clauses as sample
    print("  Sample clauses: ");
    for (int i = 0; i < std::min(5, num_clauses); ++i) {
        if (i > 0) print(" | ");
        print("(");
        bool first = true;
        for (const auto& lit : large_dnf.clauses[i].literals) {
            if (!first) print("&");
            first = false;
            if (lit.is_negated) print("~");
            print("x{}", lit.variable + 1);
        }
        print(")");
    }
    println(" | ...");

    int exact_large = count_satisfying_assignments_exact(large_dnf, n_large);
    println("\nExact count: {} / {}", exact_large, 1 << n_large);
    println();

    // Show convergence over multiple independent runs
    println("Convergence of Karp-Luby estimator:");
    println("  {:>10s}  {:>10s}  {:>10s}  {:>10s}",
                 "Trials", "Estimate", "Error", "Rel. Err%");
    println("  {:>10s}  {:>10s}  {:>10s}  {:>10s}",
                 "------", "---------", "-----", "--------");

    for (int trials : {100, 500, 1000, 5000, 10000}) {
        // Run multiple independent trials and average
        double sum_est = 0.0;
        int num_runs = 20;
        for (int run = 0; run < num_runs; ++run) {
            sum_est += karp_luby_dnf(large_dnf, n_large, trials, rng);
        }
        double avg_est = sum_est / num_runs;
        double err = std::abs(avg_est - exact_large);
        double rel_err = (exact_large > 0)
            ? 100.0 * err / exact_large : 0.0;
        println("  {:10d}  {:10.2f}  {:10.2f}  {:10.3f}%",
                     trials, avg_est, err, rel_err);
    }

    // ---------------------------------------------------------------
    // Part 5: Dense vs sparse DNF
    // ---------------------------------------------------------------
    println("\n=== Part 5: Dense vs Sparse DNF Formulas ===\n");

    for (int sz : {2, 3, 5}) {
        DNF test_dnf = random_dnf(8, 20, sz, rng);
        int ex = count_satisfying_assignments_exact(test_dnf, 8);
        double kl = karp_luby_dnf(test_dnf, 8, 2000, rng);
        println("  clause_size={}: exact={:3d}  K-L={:8.2f}  err={:.2f}",
                     sz, ex, kl, std::abs(kl - ex));
    }

    println("\n=== DNF Counting Complete ===\n");
}

} // namespace randalgo
