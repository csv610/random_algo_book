#pragma once
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <cassert>
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Gale-Shapley Stable Marriage Algorithm
// Lecture 4 (MIT 6.856J): Stable Marriage Problem
// ---------------------------------------------------------------------------
// Given n men and n women with preference lists, find a stable matching
// (no pair prefers someone else over their current partner).
//
// Gale-Shapley (1962): O(n^2) time, produces the man-optimal stable matching.
// The men-propose version gives men the best possible stable partner;
// the women-propose version gives women the best possible stable partner.
//
// A matching is unstable if there exist (m, w) not matched to each other
// such that m prefers w to his current partner AND w prefers m to hers.

struct StableMarriageResult {
    std::vector<int> man_partner;   // man_partner[m] = woman matched to man m
    std::vector<int> woman_partner; // woman_partner[w] = man matched to woman w
    bool stable;

    void print() const {
        int n = static_cast<int>(man_partner.size());
        println("Stable Marriage (n={}):", n);
        for (int m = 0; m < n; ++m) {
            println("  Man {} <-> Woman {}", m, man_partner[m]);
        }
        println("  Stable: {}", stable ? "YES" : "NO");
    }
};

// Men-propose Gale-Shapley
// pref[m][r] = woman ranked r-th by man m (0 = best)
// pref[w][r] = man ranked r-th by woman w (0 = best)
inline StableMarriageResult gale_shapley(const std::vector<std::vector<int>>& pref) {
    int n = static_cast<int>(pref.size());
    if (n == 0) return {{}, {}, true};

    // Build women's rank: rank[w][m] = position of man m in woman w's list
    std::vector<std::vector<int>> rank(n, std::vector<int>(n));
    for (int w = 0; w < n; ++w) {
        for (int r = 0; r < n; ++r) {
            rank[w][pref[w][r]] = r;
        }
    }

    StableMarriageResult result;
    result.man_partner.assign(n, -1);
    result.woman_partner.assign(n, -1);

    // next_proposal[m] = next rank to propose to
    std::vector<int> next_proposal(n, 0);

    std::queue<int> free_men;
    for (int m = 0; m < n; ++m) {
        free_men.push(m);
    }

    while (!free_men.empty()) {
        int m = free_men.front();
        free_men.pop();

        int w = pref[m][next_proposal[m]++];
        int current = result.woman_partner[w];

        if (current == -1 || rank[w][m] < rank[w][current]) {
            // Woman w accepts m (rejects current if any)
            result.man_partner[m] = w;
            result.woman_partner[w] = m;
            if (current != -1) {
                result.man_partner[current] = -1;
                free_men.push(current);
            }
        } else {
            // Woman w rejects m; m remains free
            free_men.push(m);
        }
    }

    // Verify stability
    result.stable = true;
    for (int m = 0; m < n; ++m) {
        int w = result.man_partner[m];
        for (int r = 0; r < n; ++r) {
            int w2 = pref[m][r];
            if (w2 == w) break;
            int m2 = result.woman_partner[w2];
            if (rank[w2][m] < rank[w2][m2]) {
                result.stable = false;
                return result;
            }
        }
    }

    return result;
}

// Randomized Gale-Shapley: random tie-breaking in preference lists
// Produces a uniformly random stable matching (when preferences have ties)
inline StableMarriageResult randomized_gale_shapley(int n, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::vector<std::vector<int>> pref(n, std::vector<int>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) pref[i][j] = j;
        std::shuffle(pref[i].begin(), pref[i].end(), rng);
    }
    return gale_shapley(pref);
}

// Verify that a matching is stable given preferences
inline bool verify_stable_matching(const std::vector<std::vector<int>>& pref,
                                   const std::vector<int>& man_partner) {
    int n = static_cast<int>(pref.size());
    std::vector<std::vector<int>> rank(n, std::vector<int>(n));
    for (int w = 0; w < n; ++w) {
        for (int r = 0; r < n; ++r) {
            rank[w][pref[w][r]] = r;
        }
    }
    std::vector<int> woman_partner(n, -1);
    for (int m = 0; m < n; ++m) {
        woman_partner[man_partner[m]] = m;
    }
    for (int m = 0; m < n; ++m) {
        int w = man_partner[m];
        for (int r = 0; r < n; ++r) {
            int w2 = pref[m][r];
            if (w2 == w) break;
            int m2 = woman_partner[w2];
            if (rank[w2][m] < rank[w2][m2]) return false;
        }
    }
    return true;
}

} // namespace ral
