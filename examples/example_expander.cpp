#include "ral/expander.h"
#include <iostream>

int main() {
    using namespace ral;

    // Random 3-regular expander on 20 vertices
    println("=== Random 3-Regular Graph (n=20) ===");
    auto G = random_regular_graph(20, 3, 42);
    println("Vertices: {}, Degree: {}", G.n, G.d);

    int gap = G.spectral_gap(100);
    println("Spectral gap (d - |lambda_2|): {}", gap);
    println("Effective expansion: lambda_2 <= {}", G.d - gap);

    // Check expansion for a small set
    std::vector<bool> small_set(20, false);
    for (int i = 0; i < 5; ++i) small_set[i] = true;
    double exp_ratio = G.expansion_ratio(small_set);
    println("Expansion ratio for first 5 vertices: {:.4f}", exp_ratio);

    // Random walk
    auto walk = G.random_walk(0, 1000, 42);
    println("Random walk from vertex 0 (1000 steps):");
    for (int i = 0; i < 5; ++i) {
        println("  Vertex {}: visited {} times", i, walk[i]);
    }

    // PRG
    println("\n=== Expander-based PRG ===");
    std::vector<bool> seed = {true, false, true, true, false};
    auto prg = expander_prg(G, seed, 50);
    println("Seed length: {}, Output length: {}", prg.seed_length, prg.output.size());
    double stat_dist = prg_statistical_distance(prg.output, 100, 42);
    println("Statistical distance from uniform: {:.4f}", stat_dist);

    // Larger expander
    println("\n=== Larger Expander (n=50, d=4) ===");
    auto G2 = random_regular_graph(50, 4, 123);
    int gap2 = G2.spectral_gap(50);
    println("Spectral gap: {}", gap2);

    return 0;
}
