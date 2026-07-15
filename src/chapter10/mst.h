#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <climits>
#include <cmath>
#include <iomanip>
#include <random>
#include <chrono>
#include <queue>
#include "random_utils.h"

namespace randalgo {

// ============================================================
// Minimum Spanning Tree: Karger-Klein-Tarjan Algorithm
// ============================================================

struct Edge {
    int u, v;
    double weight;
    bool operator<(const Edge& o) const { return weight < o.weight; }
};

struct WeightedGraph {
    int n;
    std::vector<Edge> edges;

    WeightedGraph(int n) : n(n) {}

    void add_edge(int u, int v, double w) {
        edges.push_back({u, v, w});
    }
};

// ---- Union-Find (Disjoint Set Union) ----

struct DSU {
    std::vector<int> parent, rank_;

    DSU(int n) : parent(n), rank_(n, 0) {
        std::iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }

    bool unite(int x, int y) {
        x = find(x); y = find(y);
        if (x == y) return false;
        if (rank_[x] < rank_[y]) std::swap(x, y);
        parent[y] = x;
        if (rank_[x] == rank_[y]) rank_[x]++;
        return true;
    }
};

// ---- Kruskal's MST (for reference / verification) ----

double kruskal_mst(const WeightedGraph& G) {
    auto edges = G.edges;
    std::sort(edges.begin(), edges.end());

    DSU dsu(G.n);
    double total = 0;
    int count = 0;

    for (const auto& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            total += e.weight;
            count++;
            if (count == G.n - 1) break;
        }
    }
    return total;
}

// ---- Forest-Heavy Edge Detection ----

// Given a forest F (as list of edges), determine which edges of G are F-light
// An edge e = {u,v} is F-heavy if w(e) > max weight on path from u to v in F
// An edge e = {u,v} is F-light otherwise (or if u,v in different components)

// Compute the max-edge-on-path for all pairs via BFS from each vertex
// Returns a matrix max_on_path[u][v] = max weight edge on path from u to v in F
std::vector<std::vector<double>> max_on_path_in_forest(
    int n, const std::vector<Edge>& forest_edges) {

    std::vector<std::vector<std::pair<int,double>>> adj(n);
    for (const auto& e : forest_edges) {
        adj[e.u].push_back({e.v, e.weight});
        adj[e.v].push_back({e.u, e.weight});
    }

    std::vector<std::vector<double>> result(n, std::vector<double>(n, -1.0));

    for (int start = 0; start < n; start++) {
        // BFS
        std::vector<bool> visited(n, false);
        std::queue<std::pair<int,double>> q; // (vertex, max_weight_on_path)
        q.push({start, 0.0});
        visited[start] = true;
        result[start][start] = 0.0;

        while (!q.empty()) {
            auto [u, max_w] = q.front(); q.pop();
            for (auto [v, w] : adj[u]) {
                if (!visited[v]) {
                    visited[v] = true;
                    result[start][v] = std::max(max_w, w);
                    q.push({v, result[start][v]});
                }
            }
        }
    }

    return result;
}

// Find all F-light edges of G given a forest F
std::vector<Edge> find_f_light_edges(
    const WeightedGraph& G, const std::vector<Edge>& forest_edges) {

    auto max_w = max_on_path_in_forest(G.n, forest_edges);

    std::vector<Edge> light_edges;
    for (const auto& e : G.edges) {
        // Edge is F-light if its endpoints are in different components of F
        // OR if its weight <= max weight on path between endpoints in F
        if (max_w[e.u][e.v] < 0) {
            // Different components: definitely F-light
            light_edges.push_back(e);
        } else if (e.weight <= max_w[e.u][e.v] + 1e-9) {
            light_edges.push_back(e);
        }
    }
    return light_edges;
}

// ---- Sample edges randomly with probability p ----

WeightedGraph sample_graph(const WeightedGraph& G, double p) {
    WeightedGraph H(G.n);
    for (const auto& e : G.edges) {
        if (rng().coin_flip(p)) {
            H.add_edge(e.u, e.v, e.weight);
        }
    }
    return H;
}

// ---- Boruvka Step ----
// Returns the contracted graph and the edges added to the MST

struct BoruvkaResult {
    WeightedGraph contracted;
    std::vector<Edge> mst_edges;
    std::vector<int> component_of; // component_of[original_vertex] = component_id

    BoruvkaResult(int n) : contracted(n) {}
};

BoruvkaResult boruvka_step(const WeightedGraph& G) {
    int n = G.n;
    DSU dsu(n);

    // For each component, find the lightest outgoing edge
    std::vector<Edge> cheapest(n, {-1, -1, INF});
    std::vector<bool> in_mst_edge(G.edges.size(), false);

    for (int iter = 0; iter < static_cast<int>(std::log2(n)) + 1; iter++) {
        // Reset cheapest
        for (int i = 0; i < n; i++) cheapest[i] = {-1, -1, INF};

        // Find cheapest edge for each component
        for (size_t ei = 0; ei < G.edges.size(); ei++) {
            const auto& e = G.edges[ei];
            int cu = dsu.find(e.u);
            int cv = dsu.find(e.v);
            if (cu == cv) continue;

            if (e.weight < cheapest[cu].weight) {
                cheapest[cu] = e;
            }
            if (e.weight < cheapest[cv].weight) {
                cheapest[cv] = e;
            }
        }

        // Add cheapest edges
        bool any_added = false;
        for (int i = 0; i < n; i++) {
            if (cheapest[i].weight < INF && cheapest[i].u >= 0) {
                if (dsu.unite(cheapest[i].u, cheapest[i].v)) {
                    any_added = true;
                }
            }
        }

        if (!any_added) break;
    }

    // Collect MST edges found
    std::vector<Edge> mst_edges;
    // Count components
    std::vector<int> comp_map(n, -1);
    int num_components = 0;
    for (int i = 0; i < n; i++) {
        int root = dsu.find(i);
        if (comp_map[root] < 0) comp_map[root] = num_components++;
        comp_map[i] = comp_map[root];
    }

    // Build contracted graph
    WeightedGraph contracted(num_components);
    for (const auto& e : G.edges) {
        int cu = comp_map[e.u];
        int cv = comp_map[e.v];
        if (cu != cv) {
            contracted.add_edge(cu, cv, e.weight);
        }
    }

    BoruvkaResult result(num_components);
    result.contracted = contracted;
    result.mst_edges = mst_edges;
    result.component_of = comp_map;
    return result;
}

// ---- Complete Kruskal on a subgraph (for small graphs) ----

double kruskal_on_subgraph(const WeightedGraph& G) {
    if (G.n <= 1 || G.edges.empty()) return 0;
    auto edges = G.edges;
    std::sort(edges.begin(), edges.end());

    DSU dsu(G.n);
    double total = 0;
    int count = 0;

    for (const auto& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            total += e.weight;
            count++;
            if (count == G.n - 1) break;
        }
    }
    return total;
}

// ============================================================
// Karger-Klein-Tarjan MST Algorithm
// ============================================================

// Recursive KKT MST
// Returns the MST weight
double kkt_mst_rec(const WeightedGraph& G, int recursion_depth = 0) {
    int n = G.n;

    // Base case: small graph, use Kruskal directly
    if (n <= 2 || G.edges.size() <= static_cast<size_t>(n)) {
        return kruskal_on_subgraph(G);
    }

    // Limit recursion depth for safety
    if (recursion_depth > 20) {
        return kruskal_on_subgraph(G);
    }

    // Step 1: Sample each edge with probability p = 1/2
    WeightedGraph H = sample_graph(G, 0.5);

    // Step 2: Recursively find MST of H
    // (We use Kruskal on H since it's smaller; the recursion is in the full KKT)
    // For a practical implementation, we just use Kruskal on H
    // The key insight is that the F-light edges are few

    // Find MST of H via Kruskal
    auto h_edges = H.edges;
    std::sort(h_edges.begin(), h_edges.end());
    DSU dsu_h(n);
    std::vector<Edge> forest_edges;
    for (const auto& e : h_edges) {
        if (dsu_h.unite(e.u, e.v)) {
            forest_edges.push_back(e);
            if (static_cast<int>(forest_edges.size()) == n - 1) break;
        }
    }

    // Step 3: Find all F-light edges of G
    auto light_edges = find_f_light_edges(G, forest_edges);

    // Step 4: Build subgraph G' of F-light edges
    WeightedGraph G_light(n);
    for (const auto& e : light_edges) {
        G_light.add_edge(e.u, e.v, e.weight);
    }

    // Step 5: Run Boruvka steps to reduce the graph
    // Each Boruvka step reduces components by at least 2x
    // We run enough steps to make the graph small
    WeightedGraph current = G_light;
    for (int step = 0; step < 5 && current.n > 2; step++) {
        auto result = boruvka_step(current);
        current = result.contracted;
        if (current.n <= 1) break;
    }

    // Step 6: Recurse on the reduced graph
    return kruskal_on_subgraph(current);
}

// Main entry point for KKT MST
double kkt_mst(const WeightedGraph& G) {
    return kkt_mst_rec(G, 0);
}

// ============================================================
// Verification
// ============================================================

bool verify_mst(const WeightedGraph& G, double mst_weight) {
    double expected = kruskal_mst(G);
    return std::abs(mst_weight - expected) < 1e-6;
}

// ============================================================
// Graph Generators
// ============================================================

WeightedGraph random_weighted_graph(int n, double p) {
    WeightedGraph G(n);
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (rng().coin_flip(p)) {
                double w = rng().rand_int(1, 1000) / 10.0;
                G.add_edge(i, j, w);
            }
        }
    }
    // Ensure connectivity
    std::vector<int> perm(n);
    std::iota(perm.begin(), perm.end(), 0);
    rng().shuffle(perm);
    for (int i = 1; i < n; i++) {
        double w = rng().rand_int(1, 1000) / 10.0;
        G.add_edge(perm[i-1], perm[i], w);
    }
    return G;
}

WeightedGraph random_dense_graph(int n) {
    WeightedGraph G(n);
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double w = rng().rand_int(1, 1000) / 10.0;
            G.add_edge(i, j, w);
        }
    }
    return G;
}

// ============================================================
// Demonstration
// ============================================================

void demonstrate_mst() {
    std::cout << "=== Minimum Spanning Tree (Karger-Klein-Tarjan) ===\n\n";

    // Test 1: Small known graph
    std::cout << "Test 1: Small graph with known MST\n";
    {
        WeightedGraph G(6);
        G.add_edge(0, 1, 4.0);
        G.add_edge(0, 2, 3.0);
        G.add_edge(1, 2, 1.0);
        G.add_edge(1, 3, 2.0);
        G.add_edge(2, 3, 8.0);
        G.add_edge(2, 4, 5.0);
        G.add_edge(3, 4, 7.0);
        G.add_edge(3, 5, 6.0);
        G.add_edge(4, 5, 9.0);

        double kruskal = kruskal_mst(G);
        double kkt = kkt_mst(G);

        std::cout << "  Kruskal MST weight: " << kruskal << "\n";
        std::cout << "  KKT MST weight:     " << kkt << "\n";
        std::cout << "  Match: " << (std::abs(kruskal - kkt) < 1e-6 ? "YES" : "NO") << "\n";

        std::cout << "\n  MST edges (Kruskal):\n";
        auto edges = G.edges;
        std::sort(edges.begin(), edges.end());
        DSU dsu(G.n);
        double total = 0;
        for (const auto& e : edges) {
            if (dsu.unite(e.u, e.v)) {
                std::cout << "    (" << e.u << "," << e.v << ") w=" << e.weight << "\n";
                total += e.weight;
            }
        }
        std::cout << "  Total: " << total << "\n";
    }

    // Test 2: Timing comparison
    std::cout << "\nTest 2: Timing comparison on random graphs\n";
    for (int n : {50, 100, 200, 500}) {
        double p = std::min(1.0, 6.0 / n);
        auto G = random_weighted_graph(n, p);

        auto t1 = std::chrono::high_resolution_clock::now();
        [[maybe_unused]] double kruskal_w = kruskal_mst(G);
        auto t2 = std::chrono::high_resolution_clock::now();
        double kkt_w = kkt_mst(G);
        auto t3 = std::chrono::high_resolution_clock::now();

        double kruskal_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        double kkt_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

        bool match = verify_mst(G, kkt_w);
        std::cout << "  n=" << n << " m=" << G.edges.size()
                  << ": Kruskal=" << std::fixed << std::setprecision(1) << kruskal_ms
                  << "ms, KKT=" << kkt_ms << "ms, match=" << (match ? "YES" : "NO") << "\n";
    }

    // Test 3: Sampling lemma verification
    std::cout << "\nTest 3: Sampling lemma verification\n";
    {
        int n = 100;
        auto G = random_weighted_graph(n, 0.1);

        // Find MST of G
        auto g_edges = G.edges;
        std::sort(g_edges.begin(), g_edges.end());
        DSU dsu(G.n);
        std::vector<Edge> mst_edges;
        for (const auto& e : g_edges) {
            if (dsu.unite(e.u, e.v)) {
                mst_edges.push_back(e);
                if (static_cast<int>(mst_edges.size()) == n - 1) break;
            }
        }

        // Sample and check how many F-light edges
        int num_samples = 100;
        int total_light = 0;
        for (int s = 0; s < num_samples; s++) {
            WeightedGraph H = sample_graph(G, 0.5);
            auto h_edges = H.edges;
            std::sort(h_edges.begin(), h_edges.end());
            DSU dsu_h(n);
            std::vector<Edge> forest;
            for (const auto& e : h_edges) {
                if (dsu_h.unite(e.u, e.v)) {
                    forest.push_back(e);
                    if (static_cast<int>(forest.size()) == n - 1) break;
                }
            }
            auto light = find_f_light_edges(G, forest);
            total_light += static_cast<int>(light.size());
        }

        double avg_light = static_cast<double>(total_light) / num_samples;
        double expected_bound = n / 0.5; // n/p

        std::cout << "  n=" << n << ", p=0.5\n";
        std::cout << "  Average F-light edges: " << std::fixed << std::setprecision(1) << avg_light << "\n";
        std::cout << "  Theoretical bound (n/p): " << expected_bound << "\n";
        std::cout << "  Bound satisfied: " << (avg_light <= expected_bound + 1 ? "YES" : "NO") << "\n";
    }
}

} // namespace randalgo
