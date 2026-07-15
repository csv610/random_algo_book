#pragma once

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>

namespace chapter5 {

struct MaxSATResult {
    int clauses_satisfied;
    double ratio; // fraction of clauses satisfied
};

// Evaluate how many clauses a given assignment satisfies.
// Each clause is a vector of signed variable indices:
//   positive i means literal x_i, negative means ~x_i.
// assignment[i] is 0 or 1 for variable i (1-indexed in clause vectors,
// but stored 0-indexed here: variable j corresponds to assignment[j-1]).
MaxSATResult evaluate(const std::vector<std::vector<int>>& clauses,
                      const std::vector<int>& assignment) {
    int satisfied = 0;
    for (const auto& clause : clauses) {
        bool sat = false;
        for (int lit : clause) {
            int var = std::abs(lit) - 1; // convert to 0-indexed
            bool val = (assignment[var] == 1);
            if (lit > 0 && val) { sat = true; break; }
            if (lit < 0 && !val) { sat = true; break; }
        }
        if (sat) satisfied++;
    }
    MaxSATResult res;
    res.clauses_satisfied = satisfied;
    res.ratio = static_cast<double>(satisfied) / static_cast<double>(clauses.size());
    return res;
}

// Random assignment: each variable is 0 or 1 with probability 1/2.
// Returns result for a single random trial.
MaxSATResult random_maxsat(const std::vector<std::vector<int>>& clauses,
                           int n_vars, std::mt19937& rng) {
    std::uniform_int_distribution<int> coin(0, 1);
    std::vector<int> assignment(n_vars);
    for (int i = 0; i < n_vars; ++i)
        assignment[i] = coin(rng);
    return evaluate(clauses, assignment);
}

// Run random_maxsat over many trials and return the best result found.
MaxSATResult random_maxsat_multi(const std::vector<std::vector<int>>& clauses,
                                 int n_vars, int trials, std::mt19937& rng) {
    MaxSATResult best{0, 0.0};
    for (int t = 0; t < trials; ++t) {
        MaxSATResult r = random_maxsat(clauses, n_vars, rng);
        if (r.clauses_satisfied > best.clauses_satisfied)
            best = r;
    }
    return best;
}

// Compute conditional expectation E[X | partial assignment] for MAX-SAT.
// X = number of satisfied clauses. For each clause, if it is already
// satisfied by the partial assignment, contribute 1. If all determined
// literals are false and no unset literal remains, contribute 0.
// Otherwise, if there are r unset literals, contribute 1 - 2^{-r}.
double conditional_expectation(const std::vector<std::vector<int>>& clauses,
                               const std::vector<int>& assignment,
                               int n_vars) {
    double total = 0.0;
    for (const auto& clause : clauses) {
        bool sat = false;
        bool undecided = false;
        int unset_count = 0;
        for (int lit : clause) {
            int var = std::abs(lit) - 1;
            if (assignment[var] == -1) {
                unset_count++;
                undecided = true;
            } else {
                bool val = (assignment[var] == 1);
                bool lit_satisfied = (lit > 0 && val) || (lit < 0 && !val);
                if (lit_satisfied) { sat = true; break; }
            }
        }
        if (sat) {
            total += 1.0;
        } else if (!undecided || unset_count == 0) {
            // All literals determined and none true: contributes 0
            total += 0.0;
        } else {
            // Some unset literals remain, none of the determined ones satisfied
            total += 1.0 - std::pow(0.5, static_cast<double>(unset_count));
        }
    }
    return total;
}

// Deterministic MAX-SAT via method of conditional probabilities.
// Sets each variable to the value (0 or 1) that maximizes the conditional
// expectation of the number of satisfied clauses.
MaxSATResult deterministic_maxsat(const std::vector<std::vector<int>>& clauses,
                                  int n_vars) {
    std::vector<int> assignment(n_vars, -1); // -1 = unset

    for (int i = 0; i < n_vars; ++i) {
        // Try setting x_i = 0
        assignment[i] = 0;
        double exp_zero = conditional_expectation(clauses, assignment, n_vars);

        // Try setting x_i = 1
        assignment[i] = 1;
        double exp_one = conditional_expectation(clauses, assignment, n_vars);

        // Pick the value with higher conditional expectation
        assignment[i] = (exp_one >= exp_zero) ? 1 : 0;
    }

    return evaluate(clauses, assignment);
}

// Generate a random 3-SAT instance: n_vars variables, m clauses,
// each clause has exactly 3 random distinct literals.
std::vector<std::vector<int>> generate_random_3sat(int n_vars, int m,
                                                   std::mt19937& rng) {
    std::vector<std::vector<int>> clauses;
    std::uniform_int_distribution<int> var_dist(1, n_vars);
    std::uniform_int_distribution<int> sign_dist(0, 1);

    for (int i = 0; i < m; ++i) {
        // Pick 3 distinct variables
        std::vector<int> vars;
        while (static_cast<int>(vars.size()) < 3) {
            int v = var_dist(rng);
            if (std::find(vars.begin(), vars.end(), v) == vars.end())
                vars.push_back(v);
        }
        // Assign random signs
        std::vector<int> clause(3);
        for (int j = 0; j < 3; ++j)
            clause[j] = (sign_dist(rng) == 0) ? vars[j] : -vars[j];
        clauses.push_back(clause);
    }
    return clauses;
}

// Demonstrate: compare random and deterministic MAX-SAT
void demonstrate_maxsat() {
    std::mt19937 rng(42);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "MAX-SAT: Random Assignment vs. Deterministic (Conditional Expectation)\n";
    std::cout << std::string(66, '-') << "\n\n";

    struct TestCase { int n_vars; int m_clauses; };
    std::vector<TestCase> tests = {
        {10, 30}, {15, 50}, {20, 80}, {25, 100}, {30, 120}
    };

    std::cout << std::setw(6) << "Vars"
              << std::setw(8) << "Clauses"
              << std::setw(12) << "Rand Best"
              << std::setw(10) << "Rand Ratio"
              << std::setw(12) << "Determin."
              << std::setw(12) << "Det. Ratio"
              << std::setw(10) << "Theory"
              << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto& tc : tests) {
        auto clauses = generate_random_3sat(tc.n_vars, tc.m_clauses, rng);

        // Random: run 1000 trials, take best
        MaxSATResult rand_best = random_maxsat_multi(clauses, tc.n_vars, 1000, rng);

        // Deterministic: conditional expectation method
        MaxSATResult det = deterministic_maxsat(clauses, tc.n_vars);

        // Theory: 7/8 fraction for 3-SAT
        double theory = 7.0 / 8.0;

        std::cout << std::setw(6) << tc.n_vars
                  << std::setw(8) << tc.m_clauses
                  << std::setw(12) << rand_best.clauses_satisfied
                  << std::setw(10) << rand_best.ratio
                  << std::setw(12) << det.clauses_satisfied
                  << std::setw(12) << det.ratio
                  << std::setw(10) << theory
                  << "\n";
    }

    std::cout << "\nKey observations:\n";
    std::cout << "  - Random 3-SAT assignment satisfies >= 7/8 of clauses in expectation\n";
    std::cout << "  - Deterministic method (conditional expectations) achieves >= 7/8 always\n";
    std::cout << "  - Deterministic is at least as good as the best of many random trials\n";
    std::cout << "  - The method works by fixing variables one at a time, maintaining\n";
    std::cout << "    E[X] >= 7m/8 via the tower property of conditional expectation\n";
}

} // namespace chapter5
