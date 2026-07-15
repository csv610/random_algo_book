#pragma once
#include "mis.h"
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
#include <queue>
#include <limits>
#include <cassert>

namespace randalgo {

// ---------------------------------------------------------------------------
// Random Maximal Matching -- each edge considered independently with
// probability 1/(2d), then clean up for maximality.
// ---------------------------------------------------------------------------

inline Matching random_maximal_matching(const Graph& g) {
    std::println("\n=== Random Maximal Matching (Edge-Independent) ===");

    std::mt19937 rng{std::random_device{}()};
    std::vector<bool> used(g.n, false);
    Matching matching;

    // Phase 1: each vertex independently picks an incident edge
    // with probability 1/(2d). We simulate by vertex-driven randomness.
    std::vector<int> selected_edge(g.n, -1);  // which neighbour each vertex selects
    for (int v = 0; v < g.n; ++v) {
        int d = g.degree(v);
        if (d == 0) continue;
        double prob = 1.0 / (2.0 * d);
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(rng) < prob) {
            // Pick a random neighbour
            std::uniform_int_distribution<int> ndist(0, d - 1);
            int nb = g.adj[v][ndist(rng)];
            selected_edge[v] = nb;
        }
    }

    // Phase 2: an edge (u,v) is in the candidate matching if
    // u selected v AND v selected u (both endpoints agree)
    // OR at least one endpoint selected and the other didn't
    // For simplicity: if either endpoint selected and neither is used
    for (int v = 0; v < g.n; ++v) {
        if (selected_edge[v] == -1) continue;
        int u = selected_edge[v];
        if (used[u] || used[v]) continue;
        // Check if u also selected v (bidirectional) or if we can take it
        if (selected_edge[u] == v || selected_edge[u] == -1 || u < v) {
            matching.edges.emplace_back(std::min(u, v), std::max(u, v));
            used[u] = used[v] = true;
        }
    }

    // Phase 3: greedily add remaining edges for maximality
    std::vector<std::pair<int,int>> all_edges;
    for (int u = 0; u < g.n; ++u)
        for (int v : g.adj[u])
            if (u < v) all_edges.emplace_back(u, v);

    std::mt19937 rng2{std::random_device{}()};
    std::shuffle(all_edges.begin(), all_edges.end(), rng2);

    for (auto [u, v] : all_edges) {
        if (!used[u] && !used[v]) {
            matching.edges.emplace_back(u, v);
            used[u] = used[v] = true;
        }
    }

    matching.print();
    std::println("Is valid matching: {}",
        verify_matching(g, matching) ? "YES" : "NO");
    return matching;
}

// ---------------------------------------------------------------------------
// Maximum matching via breadth-first augmenting paths (restricted to moderate-size graphs)
// ---------------------------------------------------------------------------

inline Matching blossom_matching(const Graph& g) {
    std::println("\n=== Maximum Matching (Augmenting Paths) ===");

    int n = g.n;
    std::vector<int> match(n, -1);  // match[v] = partner, -1 if free

    // Find augmenting path using BFS from each free vertex
    auto try_augment = [&](int start) -> bool {
        std::vector<int> parent(n, -1);
        std::vector<bool> visited(n, false);
        std::queue<int> q;
        q.push(start);
        visited[start] = true;

        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : g.adj[u]) {
                if (visited[v]) continue;
                visited[v] = true;
                if (match[v] == -1) {
                    // Found augmenting path: trace back
                    int cur = u;
                    int w = v;
                    while (w != -1) {
                        int prev = parent[cur];
                        match[w] = cur;
                        match[cur] = w;
                        cur = prev;
                        w = (prev != -1) ? parent[prev] : -1;
                    }
                    return true;
                }
                // v is matched; follow matched edge
                int w = match[v];
                if (!visited[w]) {
                    parent[w] = u;
                    visited[w] = true;
                    q.push(w);
                }
            }
        }
        return false;
    };

    // Repeatedly find augmenting paths
    int aug_count = 0;
    for (int v = 0; v < n; ++v) {
        if (match[v] == -1) {
            if (try_augment(v)) {
                ++aug_count;
                std::println("  Augmentation {} found", aug_count);
            }
        }
    }

    Matching matching;
    std::vector<bool> counted(n, false);
    for (int v = 0; v < n; ++v) {
        if (match[v] != -1 && !counted[v]) {
            matching.edges.emplace_back(std::min(v, match[v]), std::max(v, match[v]));
            counted[v] = counted[match[v]] = true;
        }
    }

    matching.print();
    std::println("Is valid matching: {}",
        verify_matching(g, matching) ? "YES" : "NO");
    std::println("Maximum matching size: {}", matching.size());
    return matching;
}

// ---------------------------------------------------------------------------
// 1/2-approximate maximum matching via random greedy
// ---------------------------------------------------------------------------

inline Matching approximate_max_matching(const Graph& g) {
    std::println("\n=== 1/2-Approximate Max Matching (Random Greedy) ===");

    std::mt19937 rng{std::random_device{}()};
    int n = g.n;

    // Each vertex independently picks a random neighbour
    // with probability 1/(2d)
    std::vector<bool> in_matching(n, false);
    Matching matching;

    // Build candidate edges
    std::vector<std::pair<int,int>> candidates;
    for (int u = 0; u < n; ++u) {
        int d = g.degree(u);
        if (d == 0) continue;
        double prob = 1.0 / (2.0 * d);
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(rng) < prob) {
            std::uniform_int_distribution<int> ndist(0, d - 1);
            int v = g.adj[u][ndist(rng)];
            if (u < v)
                candidates.emplace_back(u, v);
            else
                candidates.emplace_back(v, u);
        }
    }

    // Deduplicate and shuffle
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()),
                      candidates.end());
    std::shuffle(candidates.begin(), candidates.end(), rng);

    // Select non-conflicting edges
    for (auto [u, v] : candidates) {
        if (!in_matching[u] && !in_matching[v]) {
            matching.edges.emplace_back(u, v);
            in_matching[u] = in_matching[v] = true;
        }
    }

    // Fill in greedily for maximality
    std::vector<std::pair<int,int>> all_edges;
    for (int u = 0; u < n; ++u)
        for (int v : g.adj[u])
            if (u < v) all_edges.emplace_back(u, v);
    std::shuffle(all_edges.begin(), all_edges.end(), rng);

    for (auto [u, v] : all_edges) {
        if (!in_matching[u] && !in_matching[v]) {
            matching.edges.emplace_back(u, v);
            in_matching[u] = in_matching[v] = true;
        }
    }

    matching.print();
    std::println("Is valid matching: {}",
        verify_matching(g, matching) ? "YES" : "NO");
    return matching;
}

// ---------------------------------------------------------------------------
// Demonstration
// ---------------------------------------------------------------------------

inline void demonstrate_matchings() {
    std::println("+==============================================================+");
    std::println("|   Chapter 12: Matchings -- Random & Maximum                 |");
    std::println("+==============================================================+");

    // --- Bipartite graph ---
    {
        std::println("\n* Bipartite graph K_{{3,3}}");
        // K_{3,3}: left {0,1,2}, right {3,4,5}
        Graph g(6);
        for (int i = 0; i < 3; ++i)
            for (int j = 3; j < 6; ++j)
                g.add_edge(i, j);

        auto rm = random_maximal_matching(g);
        auto bm = blossom_matching(g);
        auto am = approximate_max_matching(g);

        std::println("\n  Comparison:");
        std::println("    Random maximal:  {}", rm.size());
        std::println("    Maximum (aug):   {}", bm.size());
        std::println("    1/2-approx:      {}", am.size());
    }

    // --- Non-bipartite: odd cycle C5 ---
    {
        std::println("\n* Non-bipartite: C5");
        auto g = make_cycle(5);

        auto rm = random_maximal_matching(g);
        auto bm = blossom_matching(g);
        auto am = approximate_max_matching(g);

        std::println("\n  Comparison (C5):");
        std::println("    Random maximal:  {}", rm.size());
        std::println("    Maximum:         {}", bm.size());
        std::println("    1/2-approx:      {}", am.size());
    }

    // --- Petersen graph ---
    {
        std::println("\n* Petersen graph (10 vertices)");
        // Petersen graph: outer cycle 0-1-2-3-4, inner 5-6-7-8-9
        // cross edges: 0-5, 1-6, 2-7, 3-8, 4-9
        // inner: 5-7, 7-9, 9-6, 6-8, 8-5 (star pattern)
        Graph g(10);
        for (int i = 0; i < 5; ++i) {
            g.add_edge(i, (i + 1) % 5);           // outer cycle
            g.add_edge(i, i + 5);                  // cross edges
            g.add_edge(i + 5, ((i + 2) % 5) + 5); // inner star
        }

        auto rm = random_maximal_matching(g);
        auto bm = blossom_matching(g);
        auto am = approximate_max_matching(g);

        std::println("\n  Comparison (Petersen):");
        std::println("    Random maximal:  {}", rm.size());
        std::println("    Maximum:         {}", bm.size());
        std::println("    1/2-approx:      {}", am.size());
        std::println("    (Petersen matching number = 5)");
    }

    // --- Random graph ---
    {
        std::println("\n* Random graph G(10, 0.4)");
        auto g = make_random(10, 0.4);

        auto rm = random_maximal_matching(g);
        auto bm = blossom_matching(g);
        auto am = approximate_max_matching(g);

        std::println("\n  Comparison (random graph):");
        std::println("    Random maximal:  {}", rm.size());
        std::println("    Maximum:         {}", bm.size());
        std::println("    1/2-approx:      {}", am.size());
    }
}

} // namespace randalgo
