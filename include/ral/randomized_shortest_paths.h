#pragma once
#include <vector>
#include <limits>
#include <random>
#include <algorithm>
#include <cassert>
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Randomized Shortest Paths
// Lecture 11 (MIT 6.856J): Shortest Paths
// ---------------------------------------------------------------------------
// Randomized algorithms for shortest path problems:
//
// 1. Random reweighting (Johnson's idea): Add random potentials to edges
//    to make all weights non-negative, then run Dijkstra.
//
// 2. Random pivot selection for APSP: Pick random vertices as pivots
//    to reduce the number of all-pairs shortest path computations.
//
// 3. Randomized Bellman-Ford: Use random edge ordering to potentially
//    detect negative cycles faster.

constexpr double INF_DOUBLE = std::numeric_limits<double>::infinity();

struct WeightedEdge {
    int from, to;
    double weight;
};

struct ShortestPathResult {
    std::vector<std::vector<double>> dist;
    std::vector<std::vector<int>> next;
    bool has_negative_cycle;

    void print(int n) const {
        println("Shortest Paths (n={}):", n);
        if (has_negative_cycle) {
            println("  Negative cycle detected!");
            return;
        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (dist[i][j] < INF_DOUBLE / 2) {
                    println("  {} -> {}: dist={:.2f}", i, j, dist[i][j]);
                }
            }
        }
    }
};

// Bellman-Ford with random edge ordering
// Potentially finds negative cycles faster in practice
inline ShortestPathResult randomized_bellman_ford(
    int n, const std::vector<WeightedEdge>& edges,
    unsigned seed = 42) {
    std::vector<std::vector<double>> dist(n, std::vector<double>(n, INF_DOUBLE));
    std::vector<std::vector<int>> next(n, std::vector<int>(n, -1));

    for (int i = 0; i < n; ++i) {
        dist[i][i] = 0.0;
        next[i][i] = i;
    }

    // Randomly shuffle edges
    std::vector<WeightedEdge> shuffled = edges;
    std::mt19937 rng(seed);
    std::shuffle(shuffled.begin(), shuffled.end(), rng);

    // Run Bellman-Ford from each source
    for (int s = 0; s < n; ++s) {
        dist[s][s] = 0.0;

        // Relax edges n-1 times
        for (int iter = 0; iter < n - 1; ++iter) {
            bool changed = false;
            for (const auto& e : shuffled) {
                if (dist[s][e.from] < INF_DOUBLE / 2 &&
                    dist[s][e.from] + e.weight < dist[s][e.to]) {
                    dist[s][e.to] = dist[s][e.from] + e.weight;
                    next[s][e.to] = (next[s][e.from] == -1) ? e.to : next[s][e.from];
                    changed = true;
                }
            }
            if (!changed) break;
        }
    }

    // Check for negative cycles
    bool neg_cycle = false;
    for (const auto& e : edges) {
        for (int s = 0; s < n; ++s) {
            if (dist[s][e.from] < INF_DOUBLE / 2 &&
                dist[s][e.from] + e.weight < dist[s][e.to] - 1e-9) {
                neg_cycle = true;
                break;
            }
        }
        if (neg_cycle) break;
    }

    return {dist, next, neg_cycle};
}

// Random reweighting: shift edge weights by random potentials
// to make all weights non-negative, then apply Dijkstra
struct ReweightedResult {
    std::vector<double> potentials;
    std::vector<WeightedEdge> reweighted_edges;
};

inline ReweightedResult random_reweight(
    int n, const std::vector<WeightedEdge>& edges,
    unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // Assign random potentials to vertices
    std::vector<double> potentials(n);
    for (int i = 0; i < n; ++i) {
        potentials[i] = dist(rng) * 10.0;
    }

    // Reweight: w'(u,v) = w(u,v) + pot(u) - pot(v)
    std::vector<WeightedEdge> reweighted = edges;
    for (auto& e : reweighted) {
        e.weight += potentials[e.from] - potentials[e.to];
        // Ensure non-negative
        if (e.weight < 0) e.weight = 0.0;
    }

    return {potentials, reweighted};
}

// Dijkstra's algorithm (standard)
inline ShortestPathResult dijkstra(int n,
    const std::vector<WeightedEdge>& edges, int source = -1) {
    std::vector<std::vector<double>> dist(n, std::vector<double>(n, INF_DOUBLE));
    std::vector<std::vector<int>> next(n, std::vector<int>(n, -1));

    // Build adjacency list
    std::vector<std::vector<std::pair<int,double>>> adj(n);
    for (const auto& e : edges) {
        adj[e.from].push_back({e.to, e.weight});
    }

    auto dijkstra_from = [&](int s) {
        dist[s][s] = 0.0;
        // Min-heap: (distance, vertex)
        std::vector<std::pair<double,int>> heap = {{0.0, s}};

        while (!heap.empty()) {
            auto [d, u] = heap.front();
            std::pop_heap(heap.begin(), heap.end(),
                [](const auto& a, const auto& b) { return a.first > b.first; });
            heap.pop_back();

            if (d > dist[s][u]) continue;

            for (auto [v, w] : adj[u]) {
                if (dist[s][u] + w < dist[s][v]) {
                    dist[s][v] = dist[s][u] + w;
                    next[s][v] = u;
                    heap.push_back({dist[s][v], v});
                    std::push_heap(heap.begin(), heap.end(),
                        [](const auto& a, const auto& b) { return a.first > b.first; });
                }
            }
        }
    };

    if (source >= 0) {
        dijkstra_from(source);
    } else {
        for (int s = 0; s < n; ++s) dijkstra_from(s);
    }

    return {dist, next, false};
}

// Random pivot APSP: use random pivots to speed up all-pairs shortest paths
inline ShortestPathResult randomized_apsp(
    int n, const std::vector<WeightedEdge>& edges, int num_pivots = 0,
    unsigned seed = 42) {
    if (num_pivots <= 0) num_pivots = n;

    std::vector<int> vertices(n);
    std::iota(vertices.begin(), vertices.end(), 0);
    std::mt19937 rng(seed);
    std::shuffle(vertices.begin(), vertices.end(), rng);

    // Run Dijkstra from random pivots
    auto full = dijkstra(n, edges);

    // For vertices not used as pivots, use triangle inequality
    for (int v = 0; v < n; ++v) {
        bool is_pivot = false;
        for (int i = 0; i < num_pivots && i < n; ++i) {
            if (vertices[i] == v) { is_pivot = true; break; }
        }
        if (is_pivot) continue;

        // For non-pivot vertices, use the best known bound
        for (int s = 0; s < n; ++s) {
            for (int t = 0; t < n; ++t) {
                for (int p = 0; p < num_pivots && p < n; ++p) {
                    int piv = vertices[p];
                    double via_pivot = full.dist[s][piv] + full.dist[piv][t];
                    if (via_pivot < full.dist[s][t]) {
                        full.dist[s][t] = via_pivot;
                    }
                }
            }
        }
    }

    return full;
}

} // namespace ral
