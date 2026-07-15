#pragma once
#include <vector>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <functional>
#include <atomic>
#include <thread>
#include <latch>
#include <print>
#include <format>
#include <cmath>
#include <sstream>
#include <numeric>
#include <cassert>

namespace randalgo {

// ---------------------------------------------------------------------------
// Graph representation
// ---------------------------------------------------------------------------

struct Graph {
    int n;  // number of vertices
    std::vector<std::vector<int>> adj;

    explicit Graph(int n_) : n(n_), adj(n_) {}

    void add_edge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    int degree(int v) const { return static_cast<int>(adj[v].size()); }
};

// ---------------------------------------------------------------------------
// Independent set result
// ---------------------------------------------------------------------------

struct IndependentSet {
    std::vector<bool> in_set;

    explicit IndependentSet(int n = 0) : in_set(n, false) {}

    int size() const {
        return static_cast<int>(std::count(in_set.begin(), in_set.end(), true));
    }

    std::vector<int> vertices() const {
        std::vector<int> result;
        for (int i = 0; i < static_cast<int>(in_set.size()); ++i)
            if (in_set[i]) result.push_back(i);
        return result;
    }

    void print() const {
        auto verts = vertices();
        std::print("  IS = {{");
        for (size_t i = 0; i < verts.size(); ++i) {
            if (i) std::print(", ");
            std::print("{}", verts[i]);
        }
        std::println("}}  (size = {})", size());
    }
};

// ---------------------------------------------------------------------------
// Verify that a set is independent
// ---------------------------------------------------------------------------

inline bool verify_independent_set(const Graph& g, const IndependentSet& is) {
    auto verts = is.vertices();
    std::unordered_set<int> in_set(verts.begin(), verts.end());
    for (int v : verts) {
        for (int nb : g.adj[v]) {
            if (in_set.count(nb)) return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Verify that a set is a valid matching
// ---------------------------------------------------------------------------

struct Matching {
    std::vector<std::pair<int,int>> edges;

    int size() const { return static_cast<int>(edges.size()); }

    void print() const {
        std::print("  Matching = {{");
        for (size_t i = 0; i < edges.size(); ++i) {
            if (i) std::print(", ");
            std::print("({},{})", edges[i].first, edges[i].second);
        }
        std::println("}}  (size = {})", size());
    }
};

inline bool verify_matching(const Graph& g, const Matching& m) {
    std::vector<int> used(g.n, 0);
    for (auto [u, v] : m.edges) {
        if (u < 0 || u >= g.n || v < 0 || v >= g.n) return false;
        if (used[u]++ || used[v]++) return false;
        // Verify edge exists
        bool found = false;
        for (int nb : g.adj[u]) {
            if (nb == v) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Luby's Maximal Independent Set Algorithm (Parallel)
// ---------------------------------------------------------------------------

inline IndependentSet mis_luby_parallel(const Graph& g) {
    std::println("\n=== Luby's MIS Algorithm (Parallel) ===");

    std::mt19937 rng{std::random_device{}()};
    std::vector<bool> alive(g.n, true);
    IndependentSet mis(g.n);
    int round = 0;

    while (true) {
        ++round;
        // Count alive vertices
        int alive_count = 0;
        for (int i = 0; i < g.n; ++i)
            if (alive[i]) ++alive_count;

        if (alive_count == 0) break;

        std::println("\n--- Round {} ({} alive vertices) ---", round, alive_count);

        // Phase 1: Each alive vertex picks a random number
        std::vector<double> random_vals(g.n, -1.0);
        for (int i = 0; i < g.n; ++i) {
            if (alive[i]) {
                std::uniform_real_distribution<double> dist(0.0, 1.0);
                random_vals[i] = dist(rng);
            }
        }

        // Phase 1: Keep edge if this vertex has the larger random value
        // among alive neighbours -> candidate set
        std::vector<bool> candidate(g.n, false);
        for (int i = 0; i < g.n; ++i) {
            if (!alive[i]) continue;
            bool is_max = true;
            for (int nb : g.adj[i]) {
                if (alive[nb] && random_vals[nb] > random_vals[i]) {
                    is_max = false;
                    break;
                }
            }
            candidate[i] = is_max;
        }

        std::print("  Candidates: ");
        for (int i = 0; i < g.n; ++i)
            if (candidate[i]) std::print("{}({:.3f}) ", i, random_vals[i]);
        std::println();

        // Phase 2: Among candidates, independently add to MIS with prob 1/(2*d)
        std::vector<bool> in_mis_now(g.n, false);
        for (int i = 0; i < g.n; ++i) {
            if (!candidate[i]) continue;
            int d = std::max(1, g.degree(i));
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            double threshold = 1.0 / (2.0 * d);
            if (dist(rng) < threshold) {
                in_mis_now[i] = true;
                mis.in_set[i] = true;
            }
        }

        std::print("  Added to MIS: ");
        for (int i = 0; i < g.n; ++i)
            if (in_mis_now[i]) std::print("{} ", i);
        std::println();

        // Phase 3: Remove MIS vertices and their neighbours from alive set
        for (int i = 0; i < g.n; ++i) {
            if (!in_mis_now[i]) continue;
            alive[i] = false;
            for (int nb : g.adj[i])
                alive[nb] = false;
        }
    }

    std::println("\nFinal MIS:");
    mis.print();
    std::println("Is independent set: {}",
        verify_independent_set(g, mis) ? "YES" : "NO");
    return mis;
}

// ---------------------------------------------------------------------------
// Random Maximal Matching
// ---------------------------------------------------------------------------

inline Matching maximal_matching(const Graph& g) {
    std::println("\n=== Random Maximal Matching ===");

    std::mt19937 rng{std::random_device{}()};
    std::vector<bool> used(g.n, false);
    Matching matching;

    // Collect all edges
    std::vector<std::pair<int,int>> all_edges;
    for (int u = 0; u < g.n; ++u)
        for (int v : g.adj[u])
            if (u < v) all_edges.emplace_back(u, v);

    // Shuffle edges
    std::shuffle(all_edges.begin(), all_edges.end(), rng);

    std::println("  Processing {} edges:", all_edges.size());
    for (auto [u, v] : all_edges) {
        if (!used[u] && !used[v]) {
            matching.edges.emplace_back(u, v);
            used[u] = used[v] = true;
            std::println("    Added edge ({},{})", u, v);
        }
    }

    matching.print();
    std::println("Is valid matching: {}",
        verify_matching(g, matching) ? "YES" : "NO");
    return matching;
}

// ---------------------------------------------------------------------------
// Greedy Vertex Coloring after MIS removal
// ---------------------------------------------------------------------------

inline std::vector<int> vertex_coloring(const Graph& g) {
    std::println("\n=== Greedy Vertex Coloring ===");

    std::vector<int> color(g.n, -1);
    int num_colors = 0;

    for (int v = 0; v < g.n; ++v) {
        // Find colors used by neighbours
        std::set<int> used_colors;
        for (int nb : g.adj[v]) {
            if (color[nb] != -1)
                used_colors.insert(color[nb]);
        }
        // Assign smallest available color
        int c = 0;
        while (used_colors.count(c)) ++c;
        color[v] = c;
        num_colors = std::max(num_colors, c + 1);
        std::println("  Vertex {} -> color {}", v, c);
    }

    std::println("Total colors used: {}", num_colors);

    // Verify proper coloring
    bool proper = true;
    for (int u = 0; u < g.n; ++u) {
        for (int v : g.adj[u]) {
            if (color[u] == color[v]) { proper = false; break; }
        }
    }
    std::println("Proper coloring: {}", proper ? "YES" : "NO");
    return color;
}

// ---------------------------------------------------------------------------
// Build small test graphs
// ---------------------------------------------------------------------------

inline Graph make_path(int n) {
    Graph g(n);
    for (int i = 0; i < n - 1; ++i) g.add_edge(i, i + 1);
    return g;
}

inline Graph make_cycle(int n) {
    Graph g(n);
    for (int i = 0; i < n; ++i) g.add_edge(i, (i + 1) % n);
    return g;
}

inline Graph make_star(int center, int n_leaves) {
    int n = center + 1 + n_leaves;
    Graph g(n);
    for (int i = 1; i <= n_leaves; ++i) g.add_edge(center, center + i);
    return g;
}

inline Graph make_random(int n, double edge_prob, int seed = 42) {
    Graph g(n);
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (dist(rng) < edge_prob) g.add_edge(i, j);
    return g;
}

// ---------------------------------------------------------------------------
// Demonstration
// ---------------------------------------------------------------------------

inline void demonstrate_mis() {
    std::println("+==============================================================+");
    std::println("|   Chapter 12: Maximal Independent Set & Matching           |");
    std::println("+==============================================================+");

    // --- Path graph ---
    {
        std::println("\n* Path graph P5");
        auto g = make_path(5);
        auto mis = mis_luby_parallel(g);
        auto mm = maximal_matching(g);
    }

    // --- Cycle graph ---
    {
        std::println("\n* Cycle graph C6");
        auto g = make_cycle(6);
        auto mis = mis_luby_parallel(g);
        auto mm = maximal_matching(g);
    }

    // --- Star graph ---
    {
        std::println("\n* Star graph S(0, 4)");
        auto g = make_star(0, 4);
        auto mis = mis_luby_parallel(g);
        auto mm = maximal_matching(g);
    }

    // --- Random graph ---
    {
        std::println("\n* Random graph G(12, 0.3)");
        auto g = make_random(12, 0.3);
        auto mis = mis_luby_parallel(g);
        auto mm = maximal_matching(g);
    }

    // --- Vertex coloring ---
    {
        std::println("\n* Vertex coloring on P5");
        auto g = make_path(5);
        vertex_coloring(g);
    }
}

} // namespace randalgo
