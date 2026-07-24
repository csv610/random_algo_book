#pragma once

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <numeric>

namespace chapter7 {

inline int random_walk_step(int node, const std::vector<std::vector<int>>& adj,
                            std::mt19937& rng) {
    int d = static_cast<int>(adj[node].size());
    std::uniform_int_distribution<int> dist(0, d - 1);
    return adj[node][dist(rng)];
}

inline double mixing_time_bound(int n, double conductance) {
    if (conductance <= 0.0 || n <= 1) return -1.0;
    return std::ceil((1.0 / (conductance * conductance)) *
                     (std::log(static_cast<double>(n)) + 2.0));
}

inline double compute_conductance(const std::vector<std::vector<int>>& adj,
                                  const std::vector<int>& subset,
                                  int n_verts) {
    std::vector<bool> in_set(n_verts, false);
    for (int v : subset) in_set[v] = true;

    int total_degree = 0;
    for (int v = 0; v < n_verts; ++v)
        total_degree += static_cast<int>(adj[v].size());

    int cross_edges = 0;
    int subset_degree = 0;
    for (int v : subset) {
        subset_degree += static_cast<int>(adj[v].size());
        for (int u : adj[v])
            if (!in_set[u]) cross_edges++;
    }

    if (subset_degree == 0 || subset_degree > total_degree / 2)
        return 1.0;

    double pi_s = static_cast<double>(subset_degree) / total_degree;
    return static_cast<double>(cross_edges) / (pi_s * total_degree / 2.0);
}

inline std::vector<std::vector<int>> make_cycle(int n) {
    std::vector<std::vector<int>> adj(n);
    for (int i = 0; i < n; ++i) {
        adj[i].push_back((i + 1) % n);
        adj[i].push_back((i - 1 + n) % n);
    }
    return adj;
}

inline std::vector<std::vector<int>> make_complete(int n) {
    std::vector<std::vector<int>> adj(n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (i != j) adj[i].push_back(j);
    return adj;
}

inline std::vector<std::vector<int>> make_star(int n) {
    std::vector<std::vector<int>> adj(n);
    for (int i = 1; i < n; ++i) {
        adj[0].push_back(i);
        adj[i].push_back(0);
    }
    return adj;
}

inline std::vector<std::vector<int>> make_path(int n) {
    std::vector<std::vector<int>> adj(n);
    for (int i = 0; i < n; ++i) {
        if (i > 0) adj[i].push_back(i - 1);
        if (i < n - 1) adj[i].push_back(i + 1);
    }
    return adj;
}

inline void demonstrate_random_walk() {
    std::cout << "Random Walks on Graphs\n\n";

    std::mt19937 rng(42);

    {
        std::cout << "--- Walk on Cycle C_8 (10 steps) ---\n";
        auto adj = make_cycle(8);
        int cur = 0;
        std::cout << "  Path: " << cur;
        for (int t = 0; t < 10; ++t) {
            cur = random_walk_step(cur, adj, rng);
            std::cout << " -> " << cur;
        }
        std::cout << "\n\n";
    }

    {
        std::cout << "--- Walk on Complete Graph K_5 (10 steps) ---\n";
        auto adj = make_complete(5);
        int cur = 0;
        std::cout << "  Path: " << cur;
        for (int t = 0; t < 10; ++t) {
            cur = random_walk_step(cur, adj, rng);
            std::cout << " -> " << cur;
        }
        std::cout << "\n\n";
    }

    {
        std::cout << "--- Walk on Star K_{1,6} (10 steps) ---\n";
        auto adj = make_star(7);
        int cur = 3;
        std::cout << "  Path: " << cur;
        for (int t = 0; t < 10; ++t) {
            cur = random_walk_step(cur, adj, rng);
            std::cout << " -> " << cur;
        }
        std::cout << "\n\n";
    }

    {
        std::cout << "--- Stationary Distribution Verification ---\n";
        auto adj = make_cycle(6);
        int n = 6;
        int total_edges = 12;
        const int steps = 100000;

        std::vector<int> count(n, 0);
        int cur = 0;
        for (int t = 0; t < steps; ++t) {
            cur = random_walk_step(cur, adj, rng);
            count[cur]++;
        }

        std::cout << "  Cycle C_6, " << steps << " steps from vertex 0:\n";
        std::cout << "  Vertex:        ";
        for (int i = 0; i < n; ++i) std::cout << std::setw(8) << i;
        std::cout << "\n  Empirical:     ";
        for (int i = 0; i < n; ++i)
            std::cout << std::setw(8) << std::fixed << std::setprecision(4)
                      << static_cast<double>(count[i]) / steps;
        std::cout << "\n  Stationary:    ";
        for (int i = 0; i < n; ++i)
            std::cout << std::setw(8) << std::fixed << std::setprecision(4)
                      << static_cast<double>(2) / total_edges;
        std::cout << "\n  (pi_v = d_v / 2|E| = 2/12 = 0.1667)\n\n";
    }

    {
        std::cout << "--- Mixing Time Bounds ---\n";
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "  Graph          | n   | Conductance | Mixing Time Bound\n";
        std::cout << "  ---------------|-----|-------------|------------------\n";

        auto cycle = make_cycle(16);
        std::vector<int> half_cycle(8);
        std::iota(half_cycle.begin(), half_cycle.end(), 0);
        double phi_cycle = compute_conductance(cycle, half_cycle, 16);
        std::cout << "  Cycle C_16     |  16 |"
                  << std::setw(11) << phi_cycle << " |"
                  << std::setw(17) << mixing_time_bound(16, phi_cycle) << "\n";

        auto comp = make_complete(16);
        std::vector<int> half_comp(8);
        std::iota(half_comp.begin(), half_comp.end(), 0);
        double phi_comp = compute_conductance(comp, half_comp, 16);
        std::cout << "  Complete K_16  |  16 |"
                  << std::setw(11) << phi_comp << " |"
                  << std::setw(17) << mixing_time_bound(16, phi_comp) << "\n";

        auto star = make_star(16);
        std::vector<int> leaf_set;
        for (int i = 1; i <= 7; ++i) leaf_set.push_back(i);
        double phi_star = compute_conductance(star, leaf_set, 16);
        std::cout << "  Star K_{1,15}  |  16 |"
                  << std::setw(11) << phi_star << " |"
                  << std::setw(17) << mixing_time_bound(16, phi_star) << "\n";

        auto path = make_path(16);
        std::vector<int> half_path(8);
        std::iota(half_path.begin(), half_path.end(), 0);
        double phi_path = compute_conductance(path, half_path, 16);
        std::cout << "  Path P_16      |  16 |"
                  << std::setw(11) << phi_path << " |"
                  << std::setw(17) << mixing_time_bound(16, phi_path) << "\n";

        std::cout << "\n  A larger conductance implies a smaller mixing time bound.\n";
        std::cout << "  The complete graph mixes in O(1) steps; the path requires Theta(n^2).\n\n";
    }
}

} // namespace chapter7
