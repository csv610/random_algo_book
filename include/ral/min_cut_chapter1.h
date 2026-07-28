#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <climits>
#include <iostream>
#include <cmath>
#include "random_utils.h"

namespace ral {

// Graph represented as adjacency list with edge multiplicities
struct KargerGraph {
    int n;
    std::unordered_map<int, std::unordered_map<int, int>> adj;

    KargerGraph(int n) : n(n) {}

    void add_edge(int u, int v) {
        adj[u][v]++;
        adj[v][u]++;
    }

    int edge_count() const {
        int count = 0;
        for (const auto& [u, neighbors] : adj) {
            for (const auto& [v, w] : neighbors) {
                if (u < v) count += w;
            }
        }
        return count;
    }

    std::vector<std::pair<int, int>> get_edges() const {
        std::vector<std::pair<int, int>> edges;
        for (const auto& [u, neighbors] : adj) {
            for (const auto& [v, w] : neighbors) {
                if (u < v) {
                    for (int i = 0; i < w; i++) {
                        edges.push_back({u, v});
                    }
                }
            }
        }
        return edges;
    }
};

// Karger's Min-Cut Algorithm
inline int karger_min_cut(KargerGraph& graph) {
    int n = graph.n;

    std::unordered_map<int, std::unordered_map<int, int>> adj = graph.adj;
    std::vector<int> vertices(n);
    std::iota(vertices.begin(), vertices.end(), 0);

    while (vertices.size() > 2) {
        auto edges = KargerGraph(n);
        edges.adj = adj;
        auto all_edges = edges.get_edges();

        if (all_edges.empty()) break;

        int edge_idx = rng().rand_int(0, all_edges.size() - 1);
        auto [u, v] = all_edges[edge_idx];

        for (const auto& [w, weight] : adj[v]) {
            if (w != u) {
                adj[u][w] += weight;
                adj[w][u] += weight;
            }
        }

        adj.erase(v);
        for (auto& [w, neighbors] : adj) {
            neighbors.erase(v);
        }

        adj[u].erase(u);

        vertices.erase(std::remove(vertices.begin(), vertices.end(), v), vertices.end());
    }

    if (vertices.size() == 2) {
        return adj[vertices[0]][vertices[1]];
    }
    return 0;
}

inline int karger_min_cut_repeated(KargerGraph& graph, int num_trials) {
    int min_cut = INT_MAX;

    for (int i = 0; i < num_trials; i++) {
        int cut = karger_min_cut(graph);
        min_cut = std::min(min_cut, cut);
    }

    return min_cut;
}

inline KargerGraph create_example_graph() {
    KargerGraph g(5);
    g.add_edge(0, 1);
    g.add_edge(0, 2);
    g.add_edge(0, 3);
    g.add_edge(1, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 3);
    g.add_edge(2, 4);
    g.add_edge(3, 4);
    return g;
}

inline void demonstrate_karger_basic() {
    std::cout << "=== Karger's Min-Cut Algorithm ===\n\n";

    KargerGraph g = create_example_graph();

    std::cout << "Graph with " << g.n << " vertices and " << g.edge_count() << " edges\n";
    std::cout << "Expected min-cut size: 2\n\n";

    int single_run = karger_min_cut(g);
    std::cout << "Single run result: " << single_run << "\n";

    int n = g.n;
    int num_trials = (n * n) / 2;
    int min_cut = karger_min_cut_repeated(g, num_trials);
    std::cout << "After " << num_trials << " trials (n^2/2): " << min_cut << "\n";

    double prob_success = 2.0 / (n * (n - 1));
    double prob_failure_one = 1.0 - prob_success;
    double prob_failure_all = std::pow(prob_failure_one, num_trials);

    std::cout << "\nTheoretical Analysis:\n";
    std::cout << "P(success in one trial) >= 2/n^2 = " << prob_success << "\n";
    std::cout << "P(failure in one trial) <= " << prob_failure_one << "\n";
    std::cout << "P(failure in all " << num_trials << " trials) <= " << prob_failure_all << "\n";
    std::cout << "This is less than 1/e = " << 1.0 / std::exp(1.0) << "\n\n";
}

} // namespace ral
