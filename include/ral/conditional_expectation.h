#pragma once
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <functional>
#include <cassert>
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Method of Conditional Expectations / Probabilities
// Lecture 8 (MIT 6.856J): Method of Conditional Probabilities and Expectations
// ---------------------------------------------------------------------------
// The method of conditional expectations derandomizes a randomized algorithm
// by replacing random choices with deterministic ones based on conditional
// expectations.
//
// Given a randomized algorithm with expected cost E[C]:
//   1. Fix variables x_1, x_2, ..., x_n one at a time
//   2. For each x_i, compute E[C | x_1, ..., x_{i-1}, x_i = 0] and
//      E[C | x_1, ..., x_{i-1}, x_i = 1]
//   3. Choose x_i to minimize (or maximize) the conditional expectation
//   4. The resulting deterministic algorithm achieves cost <= E[C]
//
// Applications:
// - Derandomized MAX-SAT: achieve expected approximation ratio deterministically
// - Derandomized graph coloring
// - Derandomized set cover
// - Derandomized discrepancy minimization

// ---------------------------------------------------------------------------
// Derandomized MAX-SAT via Conditional Expectations
// ---------------------------------------------------------------------------
// Given a CNF formula, find an assignment maximizing the number of
// satisfied clauses. Random assignment satisfies E[clauses] = sum(2^{-k_i})
// where k_i is the clause length. Conditional expectations achieve at least
// this many clauses deterministically.

struct MaxSATResult {
    std::vector<bool> assignment;
    int clauses_satisfied;
    double expected_value;
    int total_clauses;
};

// A CNF clause: disjunction of literals
// positive = variable index (1-based), negative = negated variable
struct CNFClause {
    std::vector<int> literals;
};

// Evaluate how many clauses an assignment satisfies
inline int count_satisfied(const std::vector<bool>& assignment,
                           const std::vector<CNFClause>& clauses) {
    int count = 0;
    for (const auto& clause : clauses) {
        for (int lit : clause.literals) {
            int var = std::abs(lit) - 1;
            bool satisfied = (lit > 0) ? assignment[var] : !assignment[var];
            if (satisfied) { count++; break; }
        }
    }
    return count;
}

// Compute expected number of satisfied clauses given partial assignment
// variables assigned so far: first `fixed` variables are set in `assignment`
inline double conditional_expectation(const std::vector<bool>& assignment,
                                      int fixed,
                                      const std::vector<CNFClause>& clauses) {
    double expected = 0.0;

    for (const auto& clause : clauses) {
        // Check if clause is already satisfied by fixed variables
        bool already_sat = false;
        bool all_unfixed = true;
        for (int lit : clause.literals) {
            int var = std::abs(lit) - 1;
            if (var < fixed) {
                bool val = (lit > 0) ? assignment[var] : !assignment[var];
                if (val) { already_sat = true; break; }
            } else {
                all_unfixed = false;
            }
        }
        if (already_sat) {
            expected += 1.0;
        } else if (!all_unfixed) {
            // Clause not yet satisfied; some unfixed vars remain
            // Probability of being satisfied by unfixed vars
            int unfixed_count = 0;
            for (int lit : clause.literals) {
                int var = std::abs(lit) - 1;
                if (var >= fixed) unfixed_count++;
            }
            expected += 1.0 - std::pow(0.5, unfixed_count);
        }
        // If all vars are fixed and clause not satisfied, contributes 0
    }
    return expected;
}

// Derandomized MAX-SAT using method of conditional expectations
inline MaxSATResult derandomized_max_sat(const std::vector<CNFClause>& clauses,
                                          int num_vars) {
    std::vector<bool> assignment(num_vars, false);

    for (int i = 0; i < num_vars; ++i) {
        // Try x_i = 0
        assignment[i] = false;
        double exp0 = conditional_expectation(assignment, i + 1, clauses);

        // Try x_i = 1
        assignment[i] = true;
        double exp1 = conditional_expectation(assignment, i + 1, clauses);

        // Choose the one with higher conditional expectation
        if (exp0 >= exp1) {
            assignment[i] = false;
        } else {
            assignment[i] = true;
        }
    }

    int sat = count_satisfied(assignment, clauses);
    double expected = conditional_expectation(assignment, num_vars, clauses);

    return {assignment, sat, expected, static_cast<int>(clauses.size())};
}

// Random assignment for comparison
inline MaxSATResult random_max_sat(const std::vector<CNFClause>& clauses,
                                    int num_vars, int num_trials = 1000,
                                    unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 1);

    std::vector<bool> best_assignment(num_vars);
    int best_sat = 0;

    for (int t = 0; t < num_trials; ++t) {
        std::vector<bool> assignment(num_vars);
        for (int i = 0; i < num_vars; ++i) {
            assignment[i] = (dist(rng) == 1);
        }
        int sat = count_satisfied(assignment, clauses);
        if (sat > best_sat) {
            best_sat = sat;
            best_assignment = assignment;
        }
    }

    return {best_assignment, best_sat, 0.0, static_cast<int>(clauses.size())};
}

// ---------------------------------------------------------------------------
// Derandomized Discrepancy Minimization
// ---------------------------------------------------------------------------
// Given m sets over [n], find a 2-coloring of [n] that minimizes
// the maximum discrepancy (imbalance) of any set.
// Method of conditional expectations: color each element to minimize
// the expected maximum discrepancy.

struct DiscrepancyResult {
    std::vector<int> coloring; // -1 or +1
    int max_discrepancy;
};

inline DiscrepancyResult derandomized_discrepancy(
    const std::vector<std::vector<int>>& sets, int n) {
    std::vector<int> coloring(n, 1);

    for (int i = 0; i < n; ++i) {
        // For each set, compute expected |sum| given current partial coloring
        // Try color[i] = +1 and color[i] = -1

        // Compute max expected absolute sum for each choice
        auto compute_max_abs = [&](int fixed) -> double {
            double max_abs = 0.0;
            for (const auto& s : sets) {
                double sum = 0.0;
                for (int elem : s) {
                    if (elem < fixed) {
                        sum += coloring[elem];
                    }
                }
                // Expected |sum + X| where X is sum of unfixed random +/-1
                // Approximate by |sum|
                double exp_abs = std::abs(sum);
                max_abs = std::max(max_abs, exp_abs);
            }
            return max_abs;
        };

        coloring[i] = 1;
        double exp1 = compute_max_abs(i + 1);
        coloring[i] = -1;
        double exp_minus1 = compute_max_abs(i + 1);

        coloring[i] = (exp1 <= exp_minus1) ? 1 : -1;
    }

    // Compute actual max discrepancy
    int max_disc = 0;
    for (const auto& s : sets) {
        int sum = 0;
        for (int elem : s) sum += coloring[elem];
        max_disc = std::max(max_disc, std::abs(sum));
    }

    return {coloring, max_disc};
}

} // namespace ral
