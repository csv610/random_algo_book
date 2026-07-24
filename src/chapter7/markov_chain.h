#pragma once

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <numeric>

namespace chapter7 {

struct MarkovChain {
    int n;
    std::vector<std::vector<double>> P;
    std::vector<double> pi;

    MarkovChain(int n, const std::vector<std::vector<double>>& trans)
        : n(n), P(trans), pi(n, 0.0) {
        compute_stationary();
    }

    void compute_stationary() {
        std::vector<double> v(n, 1.0 / n);
        for (int iter = 0; iter < 10000; ++iter) {
            std::vector<double> nv(n, 0.0);
            for (int j = 0; j < n; ++j)
                for (int i = 0; i < n; ++i)
                    nv[j] += v[i] * P[i][j];
            double diff = 0.0;
            for (int i = 0; i < n; ++i)
                diff += std::abs(nv[i] - v[i]);
            v = nv;
            if (diff < 1e-12) break;
        }
        pi = v;
    }

    double stationary(int v) const { return pi[v]; }

    double tv_distance(const std::vector<double>& dist) const {
        double d = 0.0;
        for (int i = 0; i < n; ++i)
            d += std::abs(dist[i] - pi[i]);
        return d / 2.0;
    }

    std::vector<double> step(const std::vector<double>& dist) const {
        std::vector<double> nd(n, 0.0);
        for (int j = 0; j < n; ++j)
            for (int i = 0; i < n; ++i)
                nd[j] += dist[i] * P[i][j];
        return nd;
    }

    int walk_step(int node, std::mt19937& rng) const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double r = dist(rng);
        double cum = 0.0;
        for (int j = 0; j < n; ++j) {
            cum += P[node][j];
            if (r <= cum) return j;
        }
        return n - 1;
    }
};

inline MarkovChain complete_graph_chain(int n) {
    std::vector<std::vector<double>> P(n, std::vector<double>(n, 1.0 / n));
    return MarkovChain(n, P);
}

inline MarkovChain lazy_rw(const std::vector<std::vector<int>>& adj) {
    int n = static_cast<int>(adj.size());
    std::vector<std::vector<double>> P(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        int d = static_cast<int>(adj[i].size());
        P[i][i] = 0.5;
        for (int j : adj[i])
            P[i][j] += 0.5 / d;
    }
    return MarkovChain(n, P);
}

inline void demonstrate_markov_chain() {
    std::cout << "Markov Chain: Random Walk on Complete Graph K_5\n\n";

    auto mc = complete_graph_chain(5);

    std::cout << "Transition matrix P:\n";
    std::cout << std::fixed << std::setprecision(3);
    for (int i = 0; i < mc.n; ++i) {
        std::cout << "  [";
        for (int j = 0; j < mc.n; ++j) {
            std::cout << std::setw(6) << mc.P[i][j];
            if (j < mc.n - 1) std::cout << ",";
        }
        std::cout << " ]\n";
    }

    std::cout << "\nStationary distribution pi: [";
    for (int i = 0; i < mc.n; ++i) {
        std::cout << std::setw(6) << mc.pi[i];
        if (i < mc.n - 1) std::cout << ",";
    }
    std::cout << " ]\n";
    std::cout << "(Expected: uniform [1/5, 1/5, 1/5, 1/5, 1/5] = [0.200])\n";

    std::mt19937 rng(42);
    std::cout << "\n--- Simulating random walk from vertex 0 ---\n";
    int cur = 0;
    std::vector<int> visit_count(5, 0);
    const int steps = 5000;
    for (int t = 0; t < steps; ++t) {
        cur = mc.walk_step(cur, rng);
        visit_count[cur]++;
    }

    std::cout << "\nAfter " << steps << " steps:\n";
    std::cout << "  Visit fractions: [";
    for (int i = 0; i < mc.n; ++i) {
        double frac = static_cast<double>(visit_count[i]) / steps;
        std::cout << std::setw(6) << frac;
        if (i < mc.n - 1) std::cout << ",";
    }
    std::cout << " ]\n";
    std::cout << "  Stationary pi:   [";
    for (int i = 0; i < mc.n; ++i) {
        std::cout << std::setw(6) << mc.pi[i];
        if (i < mc.n - 1) std::cout << ",";
    }
    std::cout << " ]\n";

    std::cout << "\n--- Convergence of distribution (start from vertex 0) ---\n";
    std::vector<double> dist(5, 0.0);
    dist[0] = 1.0;
    for (int t = 0; t <= 20; ++t) {
        double tv = mc.tv_distance(dist);
        if (t % 4 == 0 || t <= 5) {
            std::cout << "  t=" << std::setw(2) << t
                      << "  TV-distance=" << std::fixed << std::setprecision(4) << tv
                      << "  dist=[";
            for (int i = 0; i < mc.n; ++i) {
                std::cout << std::setw(5) << std::setprecision(3) << dist[i];
                if (i < mc.n - 1) std::cout << ",";
            }
            std::cout << " ]\n";
        }
        dist = mc.step(dist);
    }
    std::cout << "\n";
}

} // namespace chapter7
