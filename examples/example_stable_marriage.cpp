#include "ral/stable_marriage.h"
#include <iostream>

int main() {
    using namespace ral;

    // 4x4 stable marriage problem
    // Men's preferences (higher rank = less preferred)
    std::vector<std::vector<int>> pref = {
        {0, 1, 2, 3},  // Man 0: prefers W0 > W1 > W2 > W3
        {1, 0, 2, 3},  // Man 1
        {0, 1, 2, 3},  // Man 2
        {1, 0, 2, 3}   // Man 3
    };

    println("=== Gale-Shapley Stable Marriage ===");
    auto result = gale_shapley(pref);
    result.print();
    println("Stable: {}", result.stable ? "YES" : "NO");

    // Random preferences
    println("\n=== Random Stable Marriage (n=5) ===");
    auto random_result = randomized_gale_shapley(5, 42);
    random_result.print();
    println("Stable: {}", random_result.stable ? "YES" : "NO");

    // Verify
    std::vector<std::vector<int>> pref5(5);
    std::mt19937 rng(42);
    for (int i = 0; i < 5; ++i) {
        pref5[i] = {0, 1, 2, 3, 4};
        std::shuffle(pref5[i].begin(), pref5[i].end(), rng);
    }
    auto r5 = gale_shapley(pref5);
    println("\n5x5 result stable: {}", verify_stable_matching(pref5, r5.man_partner) ? "YES" : "NO");

    return 0;
}
