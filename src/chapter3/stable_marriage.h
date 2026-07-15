#pragma once
// Stable Marriage Problem: Gale-Shapley Proposal Algorithm
// Theorem: Always terminates with stable marriage in O(n^2) proposals

#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace chapter3 {

struct StableMarriageResult {
    std::vector<int> partner;  // partner[man] = woman
    int proposals;
    bool stable;
};

class StableMarriage {
public:
    StableMarriage(int n, std::mt19937& rng) : n_(n) {
        // Generate random preference lists
        men_prefs_.resize(n);
        women_prefs_.resize(n);
        women_rank_.resize(n, std::vector<int>(n));

        for (int i = 0; i < n; ++i) {
            men_prefs_[i].resize(n);
            for (int j = 0; j < n; ++j) men_prefs_[i][j] = j;
            std::shuffle(men_prefs_[i].begin(), men_prefs_[i].end(), rng);

            women_prefs_[i].resize(n);
            for (int j = 0; j < n; ++j) women_prefs_[i][j] = j;
            std::shuffle(women_prefs_[i].begin(), women_prefs_[i].end(), rng);

            for (int j = 0; j < n; ++j)
                women_rank_[i][women_prefs_[i][j]] = j;
        }
    }

    // Run the Proposal Algorithm
    StableMarriageResult run() {
        std::vector<int> next_proposal(n_, 0);
        std::vector<int> current_wife(n_, -1);
        std::vector<int> current_husband(n_, -1);
        int proposals = 0;

        std::vector<int> free_men(n_);
        for (int i = 0; i < n_; ++i) free_men[i] = i;

        while (!free_men.empty()) {
            int man = free_men.back();
            int woman = men_prefs_[man][next_proposal[man]++];
            ++proposals;

            if (current_husband[woman] == -1) {
                current_husband[woman] = man;
                current_wife[man] = woman;
                free_men.pop_back();
            } else if (women_rank_[woman][man] <
                       women_rank_[woman][current_husband[woman]]) {
                int old = current_husband[woman];
                current_husband[woman] = man;
                current_wife[man] = woman;
                current_wife[old] = -1;
                free_men.pop_back();
                free_men.push_back(old);
            }
        }

        return {current_wife, proposals, check_stability(current_wife)};
    }

private:
    int n_;
    std::vector<std::vector<int>> men_prefs_;
    std::vector<std::vector<int>> women_prefs_;
    std::vector<std::vector<int>> women_rank_;

    bool check_stability(const std::vector<int>& wife) {
        // For each man m, check if there's a woman w he prefers over wife[m]
        // who also prefers m over her husband
        std::vector<int> husband(n_, -1);
        for (int m = 0; m < n_; ++m)
            husband[wife[m]] = m;

        for (int m = 0; m < n_; ++m) {
            for (int wi = 0; wi < n_; ++wi) {
                int w = men_prefs_[m][wi];
                if (w == wife[m]) break;

                int m_curr = husband[w];
                if (women_rank_[w][m] < women_rank_[w][m_curr])
                    return false;
            }
        }
        return true;
    }
};

// Coupon-Collector Algorithm: each man proposes uniformly at random (ignoring past rejections)
int amnesiac_algorithm(int n, int max_proposals, std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, n - 1);
    std::vector<bool> woman_received(n, false);
    int proposals = 0;
    int women_with_proposals = 0;

    while (proposals < max_proposals && women_with_proposals < n) {
        int woman = dist(rng);
        ++proposals;
        if (!woman_received[woman]) {
            woman_received[woman] = true;
            ++women_with_proposals;
        }
    }
    return proposals;
}

void demonstrate_stable_marriage() {
    std::cout << "Stable Marriage Problem (Section 3.5)\n";
    std::cout << "Gale-Shapley Proposal Algorithm\n\n";

    std::mt19937 rng(42);

    std::cout << "Experiment: Average proposals vs n*ln(n)\n\n";
    std::cout << "  n      avg_proposals   n*ln(n)    ratio\n";

    for (int n : {4, 8, 16, 32, 64, 128}) {
        double total_proposals = 0;
        int trials = 500;
        bool all_stable = true;

        for (int t = 0; t < trials; ++t) {
            StableMarriage sm(n, rng);
            auto result = sm.run();
            total_proposals += result.proposals;
            if (!result.stable) all_stable = false;
        }

        double avg = total_proposals / trials;
        double theory = n * std::log(n);
        std::cout << "  " << std::setw(3) << n
                  << "    " << std::setw(10) << std::fixed << std::setprecision(1) << avg
                  << "     " << std::setw(8) << theory
                  << "   " << std::setw(5) << std::setprecision(2) << (avg / theory)
                  << "  " << (all_stable ? "OK" : "FAIL") << "\n";
    }
    std::cout << std::defaultfloat << "\n";

    // Theorem 3.6: Pr[T_A > n ln n + cn] -> 1 - e^{-e^{-c}}
    std::cout << "Theorem 3.6: Sharp threshold for Coupon Collector connection\n";
    std::cout << "  Pr[T_A > n ln n + cn] -> 1 - e^{-e^{-c}} as n -> infinity\n\n";

    std::cout << "Amnesiac Algorithm (coupon collector connection):\n";
    std::mt19937 rng2(42);
    for (int n : {10, 50, 100}) {
        double total = 0;
        int trials = 1000;
        for (int t = 0; t < trials; ++t)
            total += amnesiac_algorithm(n, 10 * n * std::log(n), rng2);
        double avg = total / trials;
        std::cout << "  n=" << n << "  avg_amnesiac_proposals=" << avg
                  << "  n*H_n=" << (n * (std::log(n) + 0.5772)) << "\n";
    }
    std::cout << "\n";
}

}  // namespace chapter3
