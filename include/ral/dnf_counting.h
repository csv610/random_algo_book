#pragma once
#include <vector>
#include <random>
#include <cmath>
#include <cassert>
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Approximate DNF Counting (Karp-Luby Algorithm)
// Lecture 18 (MIT 6.856J): DNF Counting
// ---------------------------------------------------------------------------
// Given a Boolean formula in Disjunctive Normal Form (DNF):
//   phi = C1 OR C2 OR ... OR Cm
// where each clause Ci = (literal_1 AND literal_2 AND ... AND literal_k),
// estimate the number of satisfying assignments.
//
// Exact counting is #P-complete. Karp-Luby (1983) gives a fully
// polynomial randomized approximation scheme (FPRAS) using O(m/epsilon^2)
// samples.
//
// Key idea: Let Si = set of assignments satisfying clause Ci.
//   |S1 OR ... OR Sm| = sum |Si| - sum |Si AND Sj| + ... (inclusion-exclusion)
// Karp-Luby samples from the union distribution: pick a clause Ci
// proportional to |Si|, then pick a random assignment from Si,
// and check if it satisfies phi.

struct DNFClause {
    std::vector<int> literals; // positive = variable index, negative = negated
    // e.g., {1, -2, 3} means x1 AND NOT x2 AND x3
};

struct DNFCountingResult {
    double estimate;
    double exact_count;
    double relative_error;
    int num_samples;
    bool within_tolerance;
};

// Count satisfying assignments for a single clause
// Each clause of length k has 2^(n-k) satisfying assignments
inline double clause_size(int num_vars, const DNFClause& clause) {
    return std::pow(2.0, num_vars - static_cast<double>(clause.literals.size()));
}

// Check if an assignment satisfies a clause
inline bool satisfies_clause(const std::vector<bool>& assignment,
                             const DNFClause& clause) {
    for (int lit : clause.literals) {
        int var = std::abs(lit) - 1; // 0-indexed
        bool is_pos = (lit > 0);
        if (assignment[var] != is_pos) return false;
    }
    return true;
}

// Check if an assignment satisfies the entire DNF
inline bool satisfies_dnf(const std::vector<bool>& assignment,
                          const std::vector<DNFClause>& clauses) {
    for (const auto& c : clauses) {
        if (satisfies_clause(assignment, c)) return true;
    }
    return false;
}

// Exact counting by enumeration (for small n)
inline double exact_dnf_count(int num_vars, const std::vector<DNFClause>& clauses) {
    double count = 0;
    long long total = 1LL << num_vars;
    for (long long mask = 0; mask < total; ++mask) {
        std::vector<bool> assignment(num_vars);
        for (int i = 0; i < num_vars; ++i) {
            assignment[i] = (mask >> i) & 1;
        }
        if (satisfies_dnf(assignment, clauses)) count++;
    }
    return count;
}

// Karp-Luby FPRAS for DNF counting
// Returns an estimate of |{x : phi(x) = 1}| within (1 +/- epsilon) w.p. >= 3/4
inline DNFCountingResult karp_luby_dnf(int num_vars,
                                        const std::vector<DNFClause>& clauses,
                                        double epsilon = 0.1,
                                        unsigned seed = 42) {
    int m = static_cast<int>(clauses.size());
    if (m == 0) return {0.0, 0.0, 0.0, 0, true};
    if (num_vars == 0) return {1.0, 1.0, 0.0, 0, true};

    // Compute clause sizes and total
    std::vector<double> sizes(m);
    double total_size = 0.0;
    for (int i = 0; i < m; ++i) {
        sizes[i] = clause_size(num_vars, clauses[i]);
        total_size += sizes[i];
    }

    // Number of samples needed: O(m / epsilon^2)
    int num_samples = static_cast<int>(std::ceil(2.0 * m / (epsilon * epsilon)));

    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    // Build CDF for sampling clause proportional to size
    std::vector<double> cdf(m);
    cdf[0] = sizes[0] / total_size;
    for (int i = 1; i < m; ++i) {
        cdf[i] = cdf[i - 1] + sizes[i] / total_size;
    }

    double sum = 0.0;
    for (int s = 0; s < num_samples; ++s) {
        // Sample clause Ci proportional to |Si|
        double r = uniform(rng);
        int ci = 0;
        for (int i = 0; i < m; ++i) {
            if (r <= cdf[i]) { ci = i; break; }
        }

        // Sample random assignment satisfying Ci
        std::vector<bool> assignment(num_vars, true);
        // Set variables not in clause to random
        for (int v = 0; v < num_vars; ++v) {
            assignment[v] = (uniform(rng) < 0.5);
        }
        // Set variables in clause to their required values
        for (int lit : clauses[ci].literals) {
            int var = std::abs(lit) - 1;
            assignment[var] = (lit > 0);
        }

        // Check how many clauses this assignment satisfies
        int satisfied = 0;
        for (int j = 0; j < m; ++j) {
            if (satisfies_clause(assignment, clauses[j])) satisfied++;
        }

        sum += total_size / sizes[ci] * satisfied;
    }

    double estimate = sum / num_samples;
    double exact = exact_dnf_count(num_vars, clauses);
    double rel_err = (exact > 0) ? std::abs(estimate - exact) / exact : 0.0;

    return {estimate, exact, rel_err, num_samples, rel_err <= epsilon};
}

} // namespace ral
