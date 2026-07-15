#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <climits>
#include <cmath>
#include <iomanip>
#include <random>
#include <chrono>
#include <queue>
#include <print>
#include <format>
#include <ranges>
#include "random_utils.h"

namespace randalgo {

// ============================================================
// Minimum Cut: Karger's Algorithm & Karger-Stein
// ============================================================

struct Multigraph {
    int n;
    std::unordered_map<int, std::unordered_map<long long, int>> adj;

    explicit Multigraph(int n) : n(n) {}

    static long long make_key(int u, int v) {
        return static_cast<long long>(std::min(u, v)) * 1000000LL + std::max(u, v);
    }

    void add_edge(int u, int v) {
        if (u == v) return;
        long long key = make_key(u, v);
        adj[u][key]++;
        adj[v][key]++;
    }

    [[nodiscard]] int get_mult(int u, int v) const {
        auto it = adj.find(u);
        if (it == adj.end()) return 0;
        auto it2 = it->second.find(make_key(u, v));
        return it2 == it->second.end() ? 0 : it2->second;
    }

    [[nodiscard]] std::vector<std::pair<int, int>> all_edges() const {
        std::vector<std::pair<int, int>> edges;
        for (const auto& [u, nbrs] : adj)
            for (const auto& [key, mult] : nbrs) {
                int v = static_cast<int>(key % 1000000LL);
                if (u < v)
                    for ([[maybe_unused]] int i : std::views::iota(0, mult))
                        edges.emplace_back(u, v);
            }
        return edges;
    }

    [[nodiscard]] int total_edges() const {
        int count = 0;
        for (const auto& [u, nbrs] : adj)
            for (const auto& [key, mult] : nbrs) {
                int v = static_cast<int>(key % 1000000LL);
                if (u < v) count += mult;
            }
        return count;
    }
};

// Contract edge (u, v): merge v into u
void contract_edge(Multigraph& G, int u, int v) {
    // Transfer all edges from v to u
    if (G.adj.contains(v)) {
        for (const auto& [key, mult] : G.adj[v]) {
            int w1 = static_cast<int>(key / 1000000LL);
            int w2 = static_cast<int>(key % 1000000LL);
            int w = (w1 == v) ? w2 : w1;
            if (w == u) continue;  // skip self-loops

            long long new_key = Multigraph::make_key(u, w);
            G.adj[u][new_key] += mult;
            G.adj[w][new_key] += mult;
        }
    }

    // Remove v from all adjacency lists
    G.adj.erase(v);
    for (auto& [node, nbrs] : G.adj)
        nbrs.erase(Multigraph::make_key(u, v));

    G.n--;
}

// ============================================================
// Karger's Contraction Algorithm
// ============================================================

int karger_min_cut_size(Multigraph G) {
    while (G.n > 2) {
        auto edges = G.all_edges();
        if (edges.empty()) break;

        auto [u, v] = edges[rng().rand_int(0, static_cast<int>(edges.size()) - 1)];
        contract_edge(G, u, v);
    }

    return static_cast<int>(G.all_edges().size());
}

int karger_repeated(Multigraph graph, int num_trials) {
    return std::ranges::min(
        std::views::iota(0, num_trials)
        | std::views::transform([&](int) { return karger_min_cut_size(graph); })
    );
}

// ============================================================
// Karger-Stein Recursive Contraction
// ============================================================

int karger_stein_rec(Multigraph G, int threshold = 6) {
    if (G.n <= threshold)
        return karger_min_cut_size(G);

    int target = static_cast<int>(std::ceil(G.n / std::sqrt(2.0))) + 1;
    int contractions = G.n - target;

    Multigraph G1 = G;
    for ([[maybe_unused]] int _ : std::views::iota(0, contractions)) {
        if (G1.n <= target) break;
        auto edges = G1.all_edges();
        if (edges.empty()) break;
        auto [u, v] = edges[rng().rand_int(0, static_cast<int>(edges.size()) - 1)];
        contract_edge(G1, u, v);
    }

    Multigraph G2 = G1;  // independent copy for second call
    return std::min(karger_stein_rec(G1, threshold),
                    karger_stein_rec(G2, threshold));
}

int karger_stein(Multigraph graph, int repetitions = 1) {
    return std::ranges::min(
        std::views::iota(0, repetitions)
        | std::views::transform([&](int) { return karger_stein_rec(graph); })
    );
}

// ============================================================
// Ground Truth: Max-Flow for Verification
// ============================================================

int max_flow_bfs(const std::vector<std::vector<int>>& capacity, int s, int t) {
    int n = static_cast<int>(capacity.size());
    auto residual = capacity;
    int total_flow = 0;

    while (true) {
        std::vector<int> parent(n, -1);
        std::queue<int> q;
        q.push(s);
        parent[s] = s;

        while (!q.empty() && parent[t] == -1) {
            int u = q.front(); q.pop();
            for (int v : std::views::iota(0, n))
                if (parent[v] == -1 && residual[u][v] > 0) {
                    parent[v] = u;
                    q.push(v);
                }
        }

        if (parent[t] == -1) break;

        int path_flow = INT_MAX;
        for (int v = t; v != s; v = parent[v])
            path_flow = std::min(path_flow, residual[parent[v]][v]);
        for (int v = t; v != s; v = parent[v]) {
            residual[parent[v]][v] -= path_flow;
            residual[v][parent[v]] += path_flow;
        }
        total_flow += path_flow;
    }
    return total_flow;
}

int exact_min_cut(const Multigraph& G, int num_vertices) {
    std::vector<std::vector<int>> cap(num_vertices, std::vector<int>(num_vertices, 0));

    for (const auto& [u, nbrs] : G.adj)
        for (const auto& [key, mult] : nbrs) {
            int v = static_cast<int>(key % 1000000LL);
            if (u < v) cap[u][v] = cap[v][u] = mult;
        }

    return std::ranges::min(
        std::views::iota(1, num_vertices)
        | std::views::transform([&](int t) { return max_flow_bfs(cap, 0, t); })
    );
}

// ============================================================
// Graph Generators
// ============================================================

Multigraph random_multigraph(int n, int num_edges) {
    Multigraph G(n);
    for ([[maybe_unused]] int _ : std::views::iota(0, num_edges)) {
        int u = rng().rand_int(0, n - 1);
        int v = rng().rand_int(0, n - 2);
        if (v >= u) v++;
        G.add_edge(u, v);
    }
    return G;
}

// ============================================================
// Demonstration
// ============================================================

void demonstrate_min_cut() {
    std::println("=== Minimum Cut Algorithms ===\n");

    // Test 1: Small known graph
    std::println("Test 1: Small graph (known min-cut)");
    {
        Multigraph G(6);
        G.add_edge(0, 1); G.add_edge(0, 2); G.add_edge(0, 3);
        G.add_edge(1, 2); G.add_edge(1, 4);
        G.add_edge(2, 3); G.add_edge(2, 5);
        G.add_edge(3, 4); G.add_edge(3, 5);
        G.add_edge(4, 5);

        int exact = exact_min_cut(G, 6);
        int single = karger_min_cut_size(G);
        int n = 6;
        int trials = n * n / 2;
        int karger_rep = karger_repeated(G, trials);
        int ks = karger_stein(G, 10);

        std::println("  Exact min-cut: {}", exact);
        std::println("  Single Karger run: {}", single);
        std::println("  Karger repeated {} times: {}", trials, karger_rep);
        std::println("  Karger-Stein (10 reps): {}", ks);

        double prob = 2.0 / (n * (n - 1));
        double fail_prob = std::pow(1.0 - prob, trials);
        std::println("  P(success one run) >= {:.6f}", prob);
        std::println("  P(failure all {} runs) <= {:.6f}\n", trials, fail_prob);
    }

    // Test 2: Timing comparison
    std::println("Test 2: Timing comparison on random graphs");
    for (int n : {20, 50, 100}) {
        int m = 3 * n;
        auto G = random_multigraph(n, m);

        auto t1 = std::chrono::high_resolution_clock::now();
        int exact_val = exact_min_cut(G, n);
        auto t2 = std::chrono::high_resolution_clock::now();
        int karger_val = karger_repeated(G, n * n / 2);
        auto t3 = std::chrono::high_resolution_clock::now();
        int ks_val = karger_stein(G, static_cast<int>(std::log(n) * std::log(n)) + 1);
        auto t4 = std::chrono::high_resolution_clock::now();

        double exact_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        double karger_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();
        double ks_ms = std::chrono::duration<double, std::milli>(t4 - t3).count();

        std::println("  n={} m={}:", n, m);
        std::println("    Exact (max-flow): {} ({:.1f}ms)", exact_val, exact_ms);
        std::println("    Karger (n^2/2 runs): {} ({:.1f}ms)", karger_val, karger_ms);
        std::println("    Karger-Stein: {} ({:.1f}ms)", ks_val, ks_ms);
        std::println("    All match: {}\n", (exact_val == karger_val && exact_val == ks_val) ? "YES" : "NO");
    }

    // Test 3: Empirical success probability
    std::println("Test 3: Empirical success probability");
    {
        int n = 30;
        auto G = random_multigraph(n, 3 * n);
        int exact_val = exact_min_cut(G, n);

        constexpr int num_trials = 100;
        int success_karger = 0, success_ks = 0;

        for ([[maybe_unused]] int _ : std::views::iota(0, num_trials)) {
            if (karger_min_cut_size(G) == exact_val) success_karger++;
            if (karger_stein_rec(G) == exact_val) success_ks++;
        }

        double theoretical = 2.0 / (n * (n - 1));
        std::println("  n={}, exact min-cut={}", n, exact_val);
        std::println("  Theoretical P(success) per Karger run: {:.6f}", theoretical);
        std::println("  Empirical Karger success rate: {}/{} = {:.3f}",
                     success_karger, num_trials, static_cast<double>(success_karger) / num_trials);
        std::println("  Empirical Karger-Stein success rate: {}/{} = {:.3f}",
                     success_ks, num_trials, static_cast<double>(success_ks) / num_trials);
    }
}

} // namespace randalgo
