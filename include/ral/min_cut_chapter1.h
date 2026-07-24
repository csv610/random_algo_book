#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <numeric>
#include "random_utils.h"

namespace chapter1 {
using randalgo::rng;

// Graph represented as adjacency list with edge multiplicities
struct Graph {
    int n;  // number of vertices
    std::unordered_map<int, std::unordered_map<int, int>> adj;  // adj[u][v] = edge count
    
    Graph(int n) : n(n) {}
    
    void add_edge(int u, int v) {
        adj[u][v]++;
        adj[v][u]++;
    }
    
    int edge_count() const {
        int count = 0;
        for (const auto& [u, neighbors] : adj) {
            for (const auto& [v, w] : neighbors) {
                if (u < v) count += w;  // count each unordered pair once
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
// Returns the size of the min-cut found
int karger_min_cut(Graph& graph) {
    int n = graph.n;
    
    // Create a working copy with vertex labels
    std::unordered_map<int, std::unordered_map<int, int>> adj = graph.adj;
    std::vector<int> vertices(n);
    std::iota(vertices.begin(), vertices.end(), 0);
    
    // Contract edges until only 2 vertices remain
    while (vertices.size() > 2) {
        // Select a uniformly random edge
        auto edges = Graph(n);
        edges.adj = adj;
        auto all_edges = edges.get_edges();
        
        if (all_edges.empty()) break;
        
        int edge_idx = rng().rand_int(0, all_edges.size() - 1);
        auto [u, v] = all_edges[edge_idx];
        
        // Merge v into u
        for (const auto& [w, weight] : adj[v]) {
            if (w != u) {
                adj[u][w] += weight;
                adj[w][u] += weight;
            }
        }
        
        // Remove v from adjacency lists
        adj.erase(v);
        for (auto& [w, neighbors] : adj) {
            neighbors.erase(v);
        }
        
        // Remove self-loops
        adj[u].erase(u);
        
        // Remove v from vertices list
        vertices.erase(std::remove(vertices.begin(), vertices.end(), v), vertices.end());
    }
    
    // Return the number of edges between the remaining two vertices
    if (vertices.size() == 2) {
        return adj[vertices[0]][vertices[1]];
    }
    return 0;
}

// Return the minimum cut over num_trials independent runs of Karger's algorithm
int karger_min_cut_repeated(Graph& graph, int num_trials) {
    int min_cut = INT_MAX;
    
    for (int i = 0; i < num_trials; i++) {
        int cut = karger_min_cut(graph);
        min_cut = std::min(min_cut, cut);
    }
    
    return min_cut;
}

// Example: Create a test graph
Graph create_example_graph() {
    // Graph from Figure 1.1 in the book
    // 5 vertices, edges: (1,2), (1,3), (1,4), (2,3), (2,4), (3,4), (3,5), (4,5)
    Graph g(5);
    g.add_edge(0, 1);  // 1-2
    g.add_edge(0, 2);  // 1-3
    g.add_edge(0, 3);  // 1-4
    g.add_edge(1, 2);  // 2-3
    g.add_edge(1, 3);  // 2-4
    g.add_edge(2, 3);  // 3-4
    g.add_edge(2, 4);  // 3-5
    g.add_edge(3, 4);  // 4-5
    return g;
}

// Demonstrate the min-cut algorithm
void demonstrate_min_cut() {
    std::cout << "=== Karger's Min-Cut Algorithm ===\n\n";
    
    Graph g = create_example_graph();
    
    std::cout << "Graph with " << g.n << " vertices and " << g.edge_count() << " edges\n";
    std::cout << "Expected min-cut size: 2\n\n";
    
    // Single run
    int single_run = karger_min_cut(g);
    std::cout << "Single run result: " << single_run << "\n";
    
    // Multiple runs (as suggested in the book: n^2/2 trials)
    int n = g.n;
    int num_trials = (n * n) / 2;
    int min_cut = karger_min_cut_repeated(g, num_trials);
    std::cout << "After " << num_trials << " trials (n^2/2): " << min_cut << "\n";
    
    // Theoretical probability analysis
    double prob_success = 2.0 / (n * (n - 1));
    double prob_failure_one = 1.0 - prob_success;
    double prob_failure_all = std::pow(prob_failure_one, num_trials);
    
    std::cout << "\nTheoretical Analysis:\n";
    std::cout << "P(success in one trial) >= 2/n^2 = " << prob_success << "\n";
    std::cout << "P(failure in one trial) <= " << prob_failure_one << "\n";
    std::cout << "P(failure in all " << num_trials << " trials) <= " << prob_failure_all << "\n";
    std::cout << "This is less than 1/e = " << 1.0 / std::exp(1.0) << "\n\n";
}

} // namespace chapter1