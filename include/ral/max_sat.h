#pragma once
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <cassert>
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// MAX-SAT: Randomized Algorithms for Maximum Satisfiability
// Lecture 7 (MIT 6.856J): Probabilistic Method, MAX SAT
// ---------------------------------------------------------------------------
// Given a CNF formula, find an assignment maximizing the number of
// satisfied clauses.
//
// Random assignment: satisfies E[#clauses] = sum(1 - 2^{-k_i}) clauses
// where k_i is the length of clause i. This gives a randomized
// approximation algorithm.
//
// For weighted MAX-SAT: clauses have weights; maximize total weight
// of satisfied clauses.
//
// Derandomization via conditional expectations (see conditional_expectation.h).

struct MaxSATClause {
    std::vector<int> literals; // positive = var index (0-based), negative = negated
    double weight;
};

struct MaxSATInstance {
    int num_vars;
    std::vector<MaxSATClause> clauses;
};

struct MaxSATSolution {
    std::vector<bool> assignment;
    double weight_satisfied;
    int count_satisfied;
    int total_clauses;
};

// Evaluate assignment on a clause
inline bool eval_clause(const std::vector<bool>& assignment,
                        const MaxSATClause& clause) {
    for (int lit : clause.literals) {
        int var = std::abs(lit);
        bool val = assignment[var];
        if (lit < 0) val = !val;
        if (val) return true;
    }
    return false;
}

// Evaluate entire formula
inline MaxSATSolution evaluate(const MaxSATInstance& inst,
                                const std::vector<bool>& assignment) {
    double weight = 0.0;
    int count = 0;
    for (const auto& c : inst.clauses) {
        if (eval_clause(assignment, c)) {
            weight += c.weight;
            count++;
        }
    }
    return {assignment, weight, count, static_cast<int>(inst.clauses.size())};
}

// Random assignment
inline MaxSATSolution random_assignment(const MaxSATInstance& inst,
                                         int num_trials = 1000,
                                         unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 1);

    MaxSATSolution best = {{}, 0.0, 0, static_cast<int>(inst.clauses.size())};

    for (int t = 0; t < num_trials; ++t) {
        std::vector<bool> assignment(inst.num_vars);
        for (int i = 0; i < inst.num_vars; ++i) {
            assignment[i] = (dist(rng) == 1);
        }
        auto sol = evaluate(inst, assignment);
        if (sol.weight_satisfied > best.weight_satisfied) {
            best = sol;
        }
    }
    return best;
}

// Randomized rounding for weighted MAX-SAT
// For each variable, set x_i = 1 with probability p_i chosen to maximize
// the expected weight of satisfied clauses
inline MaxSATSolution randomized_rounding_max_sat(const MaxSATInstance& inst,
                                                    unsigned seed = 42) {
    int n = inst.num_vars;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    // Compute optimal probability for each variable
    // For each clause, the probability it's satisfied given random assignment
    // is 1 - prod((1-p_j) for positive lits) * prod(p_j for negative lits)
    // We use a greedy approach: set each p_i to maximize expected weight

    std::vector<double> p(n, 0.5);

    // Iterate to find good probabilities
    for (int iter = 0; iter < 10; ++iter) {
        for (int i = 0; i < n; ++i) {
            // Compute expected weight with p_i = 0 and p_i = 1
            double exp0 = 0.0, exp1 = 0.0;

            for (const auto& c : inst.clauses) {
                // Compute probability clause is satisfied with current p's
                // excluding variable i
                double prob_excl = 1.0;
                bool contains_i_pos = false, contains_i_neg = false;

                for (int lit : c.literals) {
                    int var = std::abs(lit);
                    if (var == i) {
                        if (lit > 0) contains_i_pos = true;
                        else contains_i_neg = true;
                    } else {
                        if (lit > 0) prob_excl *= (1.0 - p[var]);
                        else prob_excl *= p[var];
                    }
                }

                if (contains_i_pos) {
                    // Clause satisfied by x_i = 1 or other positive literals
                    exp1 += c.weight;
                    exp0 += c.weight * (1.0 - prob_excl);
                } else if (contains_i_neg) {
                    // Clause satisfied by x_i = 0 or other literals
                    exp0 += c.weight;
                    exp1 += c.weight * (1.0 - prob_excl);
                } else {
                    // Variable i not in clause
                    double prob = 1.0 - prob_excl;
                    exp0 += c.weight * prob;
                    exp1 += c.weight * prob;
                }
            }

            p[i] = (exp1 > exp0) ? 1.0 : 0.0;
        }
    }

    // Round using computed probabilities
    std::vector<bool> assignment(n);
    for (int i = 0; i < n; ++i) {
        assignment[i] = (uniform(rng) < p[i]);
    }

    return evaluate(inst, assignment);
}

// Greedy MAX-SAT: flip the variable that increases satisfied weight most
inline MaxSATSolution greedy_max_sat(const MaxSATInstance& inst,
                                      int max_flips = 100,
                                      unsigned seed = 42) {
    int n = inst.num_vars;
    std::mt19937 rng(seed);

    std::vector<bool> assignment(n, false);
    auto current = evaluate(inst, assignment);

    for (int flip = 0; flip < max_flips; ++flip) {
        int best_var = -1;
        double best_gain = 0.0;

        for (int i = 0; i < n; ++i) {
            assignment[i] = !assignment[i];
            auto trial = evaluate(inst, assignment);
            double gain = trial.weight_satisfied - current.weight_satisfied;
            if (gain > best_gain) {
                best_gain = gain;
                best_var = i;
            }
            assignment[i] = !assignment[i]; // undo
        }

        if (best_var >= 0) {
            assignment[best_var] = !assignment[best_var];
            current = evaluate(inst, assignment);
        } else {
            break; // local optimum
        }
    }

    return current;
}

// Create a random MAX-SAT instance
inline MaxSATInstance random_max_sat_instance(int num_vars, int num_clauses,
                                               int clause_length = 3,
                                               unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> var_dist(0, num_vars - 1);
    std::uniform_int_distribution<int> sign_dist(0, 1);
    std::uniform_real_distribution<double> weight_dist(1.0, 10.0);

    MaxSATInstance inst;
    inst.num_vars = num_vars;

    for (int c = 0; c < num_clauses; ++c) {
        MaxSATClause clause;
        clause.weight = weight_dist(rng);

        // Pick clause_length distinct variables
        std::vector<int> vars;
        std::vector<int> all_vars(num_vars);
        std::iota(all_vars.begin(), all_vars.end(), 0);
        std::shuffle(all_vars.begin(), all_vars.end(), rng);
        int len = std::min(clause_length, num_vars);
        for (int i = 0; i < len; ++i) {
            int v = all_vars[i];
            int lit = (sign_dist(rng) == 0) ? v + 1 : -(v + 1);
            clause.literals.push_back(lit);
        }
        inst.clauses.push_back(clause);
    }

    return inst;
}

} // namespace ral
