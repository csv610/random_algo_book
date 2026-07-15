#pragma once

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cassert>

namespace chapter5 {

double spectral_gap(const std::vector<std::vector<int>>& adj) {
    int n = static_cast<int>(adj.size());
    if (n <= 1) return 0.0;

    std::vector<double> v(n), w(n);
    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    double norm = 0.0;
    for (int i = 0; i < n; ++i) {
        v[i] = dist(rng);
        norm += v[i] * v[i];
    }
    norm = std::sqrt(norm);
    for (int i = 0; i < n; ++i) v[i] /= norm;

    double lambda1 = 0.0;
    for (int iter = 0; iter < 200; ++iter) {
        for (int i = 0; i < n; ++i) {
            w[i] = 0.0;
            for (int j : adj[i])
                w[i] += v[j];
        }
        double dot_vw = 0.0;
        for (int i = 0; i < n; ++i)
            dot_vw += v[i] * w[i];
        lambda1 = dot_vw;

        norm = 0.0;
        for (int i = 0; i < n; ++i)
            norm += w[i] * w[i];
        norm = std::sqrt(norm);
        if (norm < 1e-12) break;
        for (int i = 0; i < n; ++i)
            v[i] = w[i] / norm;
    }

    std::vector<double> u(n);
    for (int i = 0; i < n; ++i)
        u[i] = dist(rng);
    double dot_uv = 0.0;
    for (int i = 0; i < n; ++i)
        dot_uv += u[i] * v[i];
    for (int i = 0; i < n; ++i)
        u[i] -= dot_uv * v[i];
    norm = 0.0;
    for (int i = 0; i < n; ++i)
        norm += u[i] * u[i];
    norm = std::sqrt(norm);
    if (norm < 1e-12) {
        for (int i = 0; i < n; ++i)
            u[i] = dist(rng);
        dot_uv = 0.0;
        for (int i = 0; i < n; ++i)
            dot_uv += u[i] * v[i];
        for (int i = 0; i < n; ++i)
            u[i] -= dot_uv * v[i];
        norm = 0.0;
        for (int i = 0; i < n; ++i)
            norm += u[i] * u[i];
        norm = std::sqrt(norm);
    }
    for (int i = 0; i < n; ++i)
        u[i] /= norm;

    double lambda2 = 0.0;
    for (int iter = 0; iter < 300; ++iter) {
        for (int i = 0; i < n; ++i) {
            w[i] = 0.0;
            for (int j : adj[i])
                w[i] += u[j];
        }
        double dot_wv = 0.0;
        for (int i = 0; i < n; ++i)
            dot_wv += w[i] * v[i];
        for (int i = 0; i < n; ++i)
            w[i] -= dot_wv * v[i];

        double dot_uw = 0.0;
        for (int i = 0; i < n; ++i)
            dot_uw += u[i] * w[i];
        lambda2 = dot_uw;

        norm = 0.0;
        for (int i = 0; i < n; ++i)
            norm += w[i] * w[i];
        norm = std::sqrt(norm);
        if (norm < 1e-12) break;
        for (int i = 0; i < n; ++i)
            u[i] = w[i] / norm;
    }

    return std::abs(lambda2);
}

bool is_expander(const std::vector<std::vector<int>>& adj, int d,
                 double target_lambda2) {
    double l2 = spectral_gap(adj);
    return l2 <= target_lambda2;
}

std::vector<std::vector<int>> random_regular_graph(int n, int d,
                                                   std::mt19937& rng) {
    assert(n * d % 2 == 0 && "n*d must be even");
    assert(d >= 1 && d < n);

    std::vector<int> stubs;
    stubs.reserve(n * d);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < d; ++j)
            stubs.push_back(i);

    std::vector<std::vector<int>> adj(n);
    int max_attempts = 100;
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        for (auto& a : adj) a.clear();
        std::shuffle(stubs.begin(), stubs.end(), rng);

        bool valid = true;
        for (int i = 0; i < n * d; i += 2) {
            int u = stubs[i];
            int v = stubs[i + 1];
            if (u == v) { valid = false; break; }
            bool multi = false;
            for (int w : adj[u]) {
                if (w == v) { multi = true; break; }
            }
            if (multi) { valid = false; break; }
            adj[u].push_back(v);
            adj[v].push_back(u);
        }
        if (valid) break;
        if (attempt == max_attempts - 1) {
            std::cerr << "Warning: failed to generate simple d-regular graph\n";
        }
    }
    return adj;
}

void demonstrate_expander() {
    std::mt19937 rng(42);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Expander Graphs: Random Regular Graphs and Spectral Gap\n";
    std::cout << std::string(56, '-') << "\n\n";

    std::cout << "Ramanujan bound: |lambda_2| <= 2*sqrt(d-1)\n\n";

    std::cout << std::setw(6) << "n"
              << std::setw(4) << "d"
              << std::setw(12) << "Ramanujan"
              << std::setw(12) << "|lambda_2|"
              << std::setw(10) << "Expand?"
              << "\n";
    std::cout << std::string(44, '-') << "\n";

    struct TestCase { int n; int d; };
    std::vector<TestCase> tests = {
        {20, 3}, {30, 3}, {50, 3}, {100, 3},
        {20, 4}, {30, 4}, {50, 4},
        {20, 5}, {30, 5}
    };

    for (const auto& tc : tests) {
        if (tc.n * tc.d % 2 != 0) continue;

        auto adj = random_regular_graph(tc.n, tc.d, rng);
        double l2 = spectral_gap(adj);
        double ramanujan = 2.0 * std::sqrt(static_cast<double>(tc.d - 1));
        bool expander = l2 <= ramanujan;

        std::cout << std::setw(6) << tc.n
                  << std::setw(4) << tc.d
                  << std::setw(12) << ramanujan
                  << std::setw(12) << l2
                  << std::setw(10) << (expander ? "Yes" : "No")
                  << "\n";
    }

    std::cout << "\nKey observations:\n";
    std::cout << "  - Random d-regular graphs are expanders w.h.p. for fixed d >= 3\n";
    std::cout << "  - |lambda_2| typically well below the Ramanujan bound 2*sqrt(d-1)\n";
    std::cout << "  - As n increases, the spectral gap concentrates around its expectation\n";
    std::cout << "  - The spectral gap d - |lambda_2| controls expansion via Cheeger inequality\n";
}

} // namespace chapter5
