#include "ral/randomized_shortest_paths.h"
#include <iostream>

int main() {
    using namespace ral;

    // Simple graph: 0->1 (w=1), 1->2 (w=2), 0->2 (w=5)
    std::vector<WeightedEdge> edges = {
        {0, 1, 1.0}, {1, 2, 2.0}, {0, 2, 5.0},
        {1, 0, 0.5}, {2, 1, 1.0}
    };

    println("=== Randomized Shortest Paths ===");

    // Dijkstra from vertex 0
    auto dij = dijkstra(3, edges, 0);
    println("Dijkstra from 0:");
    for (int j = 0; j < 3; ++j) {
        println("  0 -> {}: dist={:.2f}", j, dij.dist[0][j]);
    }

    // Bellman-Ford
    auto bf = randomized_bellman_ford(3, edges, 42);
    println("\nBellman-Ford:");
    for (int j = 0; j < 3; ++j) {
        println("  0 -> {}: dist={:.2f}", j, bf.dist[0][j]);
    }

    // Random reweighting
    auto rw = random_reweight(3, edges, 42);
    println("\nRandom potentials: [{:.2f}, {:.2f}, {:.2f}]",
            rw.potentials[0], rw.potentials[1], rw.potentials[2]);

    // Larger graph with negative edges
    println("\n=== Graph with Negative Weights ===");
    std::vector<WeightedEdge> neg_edges = {
        {0, 1, 2.0}, {1, 2, -3.0}, {0, 2, 5.0},
        {2, 3, 1.0}, {1, 3, 4.0}
    };
    auto bf_neg = randomized_bellman_ford(4, neg_edges, 42);
    println("Bellman-Ford from 0:");
    for (int j = 0; j < 4; ++j) {
        println("  0 -> {}: dist={:.2f}", j, bf_neg.dist[0][j]);
    }
    println("Has negative cycle: {}", bf_neg.has_negative_cycle ? "YES" : "NO");

    return 0;
}
