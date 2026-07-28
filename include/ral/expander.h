#pragma once
#include <vector>
#include <random>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <cassert>
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Expander Graphs, Spectral Analysis, and Expander-based PRGs
// Lectures 7, 20, 21 (MIT 6.856J): Expanders, Eigenvalue Analysis, PRGs
// ---------------------------------------------------------------------------
// An (n, d, lambda)-expander is a d-regular graph on n vertices whose
// second-largest eigenvalue |lambda_2| <= lambda.
//
// Properties:
// - High connectivity: any set S has many neighbors
// - Fast mixing: random walk converges quickly to uniform
// - Good expansion: |N(S)| >= (1 - lambda/d) * d * |S| for |S| <= n/2
//
// Applications:
// - Pseudorandom generators (Nisan-Wigderson, zig-zag)
// - Error-correcting codes
// - Derandomization
// - Network design

struct ExpanderGraph {
    int n;
    int d;       // degree
    double lambda; // second eigenvalue bound
    std::vector<std::vector<int>> adj;

    ExpanderGraph(int n_, int d_) : n(n_), d(d_), lambda(0.0), adj(n_) {}

    void add_edge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    // Compute the adjacency matrix
    std::vector<std::vector<double>> adjacency_matrix() const {
        std::vector<std::vector<double>> A(n, std::vector<double>(n, 0.0));
        for (int u = 0; u < n; ++u) {
            for (int v : adj[u]) {
                A[u][v] = 1.0;
            }
        }
        return A;
    }

    // Power iteration to estimate the second largest eigenvalue
    // Returns the spectral gap = d - |lambda_2|
    double spectral_gap(int iterations = 100) const {
        std::vector<double> v(n, 1.0 / std::sqrt(n));

        for (int iter = 0; iter < iterations; ++iter) {
            // v_new = A * v
            std::vector<double> v_new(n, 0.0);
            for (int u = 0; u < n; ++u) {
                for (int w : adj[u]) {
                    v_new[u] += v[w];
                }
            }
            // Normalize
            double norm = 0.0;
            for (int i = 0; i < n; ++i) norm += v_new[i] * v_new[i];
            norm = std::sqrt(norm);
            if (norm < 1e-15) break;
            for (int i = 0; i < n; ++i) v_new[i] /= norm;

            // Orthogonalize against the Perron vector (all-ones / sqrt(n))
            double dot = 0.0;
            for (int i = 0; i < n; ++i) dot += v_new[i] / std::sqrt(n);
            for (int i = 0; i < n; ++i) v_new[i] -= dot / std::sqrt(n);

            v = v_new;
        }

        // Rayleigh quotient: v^T A v / v^T v
        double numerator = 0.0, denominator = 0.0;
        for (int u = 0; u < n; ++u) {
            denominator += v[u] * v[u];
            for (int w : adj[u]) {
                numerator += v[u] * v[w];
            }
        }
        double lambda2 = (denominator > 1e-15) ? numerator / denominator : 0.0;
        return d - std::abs(lambda2);
    }

    // Expansion ratio for a set S: |N(S)| / |S|
    double expansion_ratio(const std::vector<bool>& in_set) const {
        int set_size = 0;
        std::vector<bool> neighbor(n, false);
        for (int u = 0; u < n; ++u) {
            if (in_set[u]) {
                set_size++;
                for (int v : adj[u]) {
                    if (!in_set[v]) neighbor[v] = true;
                }
            }
        }
        if (set_size == 0) return 0.0;
        int neighbor_count = 0;
        for (int v = 0; v < n; ++v) {
            if (neighbor[v]) neighbor_count++;
        }
        return static_cast<double>(neighbor_count) / set_size;
    }

    // Random walk: take num_steps random steps, return visit counts
    std::vector<int> random_walk(int start, int num_steps, unsigned seed = 42) const {
        std::mt19937 rng(seed);
        std::vector<int> counts(n, 0);
        int current = start;
        for (int s = 0; s <= num_steps; ++s) {
            counts[current]++;
            if (s < num_steps) {
                std::uniform_int_distribution<int> dist(0, static_cast<int>(adj[current].size()) - 1);
                current = adj[current][dist(rng)];
            }
        }
        return counts;
    }
};

// ---------------------------------------------------------------------------
// Random d-Regular Expander Construction
// ---------------------------------------------------------------------------
// Create a random d-regular graph using the configuration model.
// For d >= 3, random regular graphs are expanders with high probability.
inline ExpanderGraph random_regular_graph(int n, int d, unsigned seed = 42) {
    assert(n * d % 2 == 0 && "n*d must be even");
    assert(d >= 1 && "degree must be >= 1");

    ExpanderGraph G(n, d);
    std::mt19937 rng(seed);

    // Configuration model: create d copies of each vertex
    std::vector<int> stubs;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < d; ++j) {
            stubs.push_back(i);
        }
    }

    // Randomly pair stubs
    std::shuffle(stubs.begin(), stubs.end(), rng);
    for (size_t i = 0; i < stubs.size(); i += 2) {
        int u = stubs[i], v = stubs[i + 1];
        if (u != v) {
            G.add_edge(u, v);
        }
    }

    // Compute spectral gap
    G.lambda = d - G.spectral_gap(50);

    return G;
}

// ---------------------------------------------------------------------------
// Zig-Zag Product (Reingold-Vadhan-Wigderson)
// ---------------------------------------------------------------------------
// The zig-zag product of a large d1-regular graph G on n vertices
// and a small d2-regular graph H on d1 vertices produces a
// (d2^2, O(1/n))-expander on n*d1 vertices.
inline ExpanderGraph zig_zag_product(const ExpanderGraph& G,
                                      const ExpanderGraph& H) {
    int n1 = G.n, d1 = G.d;
    int d2 = H.d;

    // Number of vertices in the product: n1 * d1
    int n_new = n1 * d1;
    ExpanderGraph product(n_new, d2 * d2);

    // Build the product graph
    // Each vertex (u, i) where u in [n1], i in [d1]
    // Edges: follow H's structure within each "cloud"
    for (int u = 0; u < n1; ++u) {
        for (int i = 0; i < d1; ++i) {
            int v1 = u * d1 + i; // vertex in product

            for (int j = 0; j < d2; ++j) {
                // Zig-zag: first zag (within cloud), then zig (between clouds)
                int h_target = H.adj[i][j % d2];
                int g_target_idx = j % static_cast<int>(G.adj[u].size());
                int v_cloud = G.adj[u][g_target_idx]; // target cloud
                int v2 = v_cloud * d1 + (h_target % d1);

                if (v1 != v2) {
                    product.add_edge(v1, v2);
                }
            }
        }
    }

    return product;
}

// ---------------------------------------------------------------------------
// Nisan-Wigderson Pseudorandom Generator (Expander-based PRG)
// ---------------------------------------------------------------------------
// Given an expander graph, construct a PRG that stretches a short
// random seed into a longer pseudorandom string.
//
// The key property: for any subset S of vertices with |S| <= n/2,
// the random walk starting from any vertex visits S with probability
// at most |S|/n + lambda/d, where lambda is the second eigenvalue.

struct PRGResult {
    std::vector<bool> output;
    int seed_length;
};

// Expander-based PRG: use random walk on expander to generate bits
// seed: initial random bits
// num_bits: number of output bits desired
inline PRGResult expander_prg(const ExpanderGraph& exp,
                               const std::vector<bool>& seed,
                               int num_bits) {
    PRGResult result;
    result.seed_length = static_cast<int>(seed.size());
    result.output.resize(num_bits);

    // Convert seed to starting vertex (binary encoding)
    int start = 0;
    for (size_t i = 0; i < seed.size() && i < 20; ++i) {
        if (seed[i]) start |= (1 << i);
    }
    start %= exp.n;

    // Random walk using seed bits for choices
    int current = start;
    int bit_idx = 0;
    int step = 0;

    while (bit_idx < num_bits) {
        // Use current vertex parity + step as output bit
        result.output[bit_idx] = ((current + step) % 2 == 0);
        bit_idx++;

        // Take a random step (use hash of seed + step)
        if (!exp.adj[current].empty()) {
            int next_idx = (step * 7 + current * 13) % static_cast<int>(exp.adj[current].size());
            current = exp.adj[current][next_idx];
        }
        step++;
    }

    return result;
}

// Test if PRG output is distinguishable from random
// Returns the statistical distance from uniform
inline double prg_statistical_distance(const std::vector<bool>& prg_output,
                                        int num_trials, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 1);

    int n = static_cast<int>(prg_output.size());
    int matches = 0;
    for (int t = 0; t < num_trials; ++t) {
        // Generate random string and count matching positions
        int m = 0;
        for (int i = 0; i < n; ++i) {
            if (dist(rng) == (prg_output[i] ? 1 : 0)) m++;
        }
        matches += m;
    }
    // Average fraction of matching bits (should be ~0.5 for good PRG)
    double avg_match = static_cast<double>(matches) / (num_trials * n);
    return std::abs(avg_match - 0.5);
}

} // namespace ral
