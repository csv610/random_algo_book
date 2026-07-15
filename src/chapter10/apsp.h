#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <cmath>
#include <iomanip>
#include <random>
#include <chrono>
#include <queue>
#include <print>
#include <format>
#include <ranges>
#include <span>
#include <bit>
#include "random_utils.h"

namespace randalgo {

// ============================================================
// All-Pairs Shortest Paths
// ============================================================

inline constexpr double INF = 1e18;

using Matrix = std::vector<std::vector<double>>;
using BoolMatrix = std::vector<std::vector<bool>>;

// ---- Baseline: Floyd-Warshall O(n^3) ----

Matrix floyd_warshall(const Matrix& W) {
    int n = static_cast<int>(W.size());
    Matrix D = W;

    for (int k : std::views::iota(0, n))
        for (int i : std::views::iota(0, n))
            for (int j : std::views::iota(0, n))
                D[i][j] = std::min(D[i][j], D[i][k] + D[k][j]);

    return D;
}

// ---- Min-Plus Matrix Multiplication O(n^3) ----

Matrix min_plus_product(const Matrix& A, const Matrix& B) {
    int n = static_cast<int>(A.size());
    Matrix C(n, std::vector<double>(n, INF));

    for (int i : std::views::iota(0, n))
        for (int j : std::views::iota(0, n))
            for (int k : std::views::iota(0, n))
                C[i][j] = std::min(C[i][j], A[i][k] + B[k][j]);

    return C;
}

// ---- APSP via Repeated Min-Plus Squaring O(n^3 log n) ----

Matrix apsp_repeated_squaring(const Matrix& W) {
    int n = static_cast<int>(W.size());
    Matrix D = W;

    int steps = std::bit_width(static_cast<unsigned>(n - 1));

    for ([[maybe_unused]] int _ : std::views::iota(0, steps))
        D = min_plus_product(D, D);

    return D;
}

// ---- Boolean Matrix Multiply (for Seidel) ----

BoolMatrix bool_matrix_multiply(const BoolMatrix& A, const BoolMatrix& B) {
    int n = static_cast<int>(A.size());
    BoolMatrix C(n, std::vector<bool>(n, false));

    for (int i : std::views::iota(0, n))
        for (int j : std::views::iota(0, n))
            for (int k : std::views::iota(0, n))
                if (A[i][k] && B[k][j]) { C[i][j] = true; break; }

    return C;
}

// ---- Seidel's Algorithm for Unweighted Undirected Graphs ----

Matrix seidel_apsp_unweighted(const BoolMatrix& adj) {
    int n = static_cast<int>(adj.size());

    // Compute boolean powers iteratively: A^1, A^2, ..., A^(n-1)
    std::vector<BoolMatrix> all_powers(n);
    all_powers[0] = adj;

    for (int k : std::views::iota(1, n))
        all_powers[k] = bool_matrix_multiply(all_powers[k - 1], adj);

    Matrix dist(n, std::vector<double>(n, INF));

    for (int i : std::views::iota(0, n)) {
        dist[i][i] = 0;
        for (int j : std::views::iota(0, n)) {
            if (i == j) continue;
            for (int k : std::views::iota(0, n)) {
                if (all_powers[k][i][j]) {
                    dist[i][j] = static_cast<double>(k + 1);
                    break;
                }
            }
        }
    }

    return dist;
}

// ---- Graph Generators ----

Matrix random_weighted_graph_matrix(int n, double p) {
    Matrix W(n, std::vector<double>(n, INF));
    for (int i : std::views::iota(0, n)) W[i][i] = 0;

    for (int i : std::views::iota(0, n))
        for (int j : std::views::iota(i + 1, n))
            if (rng().coin_flip(p)) {
                double w = rng().rand_int(1, 100);
                W[i][j] = W[j][i] = w;
            }

    // Ensure connectivity via random spanning tree
    std::vector perm(n, 0);
    std::ranges::iota(perm, 0);
    rng().shuffle(perm);

    for (int i : std::views::iota(1, n)) {
        double w = rng().rand_int(1, 100);
        W[perm[i - 1]][perm[i]] = W[perm[i]][perm[i - 1]] = w;
    }

    return W;
}

BoolMatrix random_unweighted_graph(int n, double p) {
    BoolMatrix adj(n, std::vector<bool>(n, false));

    for (int i : std::views::iota(0, n))
        for (int j : std::views::iota(i + 1, n))
            if (rng().coin_flip(p))
                adj[i][j] = adj[j][i] = true;

    std::vector perm(n, 0);
    std::ranges::iota(perm, 0);
    rng().shuffle(perm);

    for (int i : std::views::iota(1, n))
        adj[perm[i - 1]][perm[i]] = adj[perm[i]][perm[i - 1]] = true;

    return adj;
}

// ---- Verification ----

bool verify_apsp(const Matrix& computed, const Matrix& expected, double eps = 0.1) {
    int n = static_cast<int>(computed.size());
    for (int i : std::views::iota(0, n))
        for (int j : std::views::iota(0, n))
            if (std::abs(computed[i][j] - expected[i][j]) > eps)
                return false;
    return true;
}

// ---- Demonstration ----

void demonstrate_apsp() {
    std::println("=== All-Pairs Shortest Paths ===\n");

    // Test 1: Small graph with known distances
    std::println("Test 1: Small weighted graph (5 vertices)");
    {
        constexpr int n = 5;
        Matrix W(n, std::vector<double>(n, INF));
        for (int i : std::views::iota(0, n)) W[i][i] = 0;

        auto add = [&](int u, int v, double w) { W[u][v] = W[v][u] = w; };
        add(0, 1, 10); add(0, 2, 3); add(1, 2, 1);
        add(1, 3, 2); add(2, 3, 8); add(2, 4, 4);
        add(3, 4, 7); add(1, 4, 6);

        std::println("  Floyd-Warshall distances:");
        auto fw = floyd_warshall(W);
        for (int i : std::views::iota(0, n)) {
            std::print("    ");
            for (int j : std::views::iota(0, n))
                std::print("{:4}", fw[i][j] >= INF ? -1 : static_cast<int>(fw[i][j]));
            std::println("");
        }

        std::println("  Min-plus squaring distances:");
        auto mps = apsp_repeated_squaring(W);
        for (int i : std::views::iota(0, n)) {
            std::print("    ");
            for (int j : std::views::iota(0, n))
                std::print("{:4}", mps[i][j] >= INF ? -1 : static_cast<int>(mps[i][j]));
            std::println("");
        }

        std::println("  Results match: {}\n", verify_apsp(mps, fw) ? "YES" : "NO");
    }

    // Test 2: Timing comparison
    std::println("Test 2: Timing comparison on random weighted graphs");
    for (int n : {50, 100, 200}) {
        auto W = random_weighted_graph_matrix(n, 0.3);

        auto t1 = std::chrono::high_resolution_clock::now();
        auto fw = floyd_warshall(W);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto mps = apsp_repeated_squaring(W);
        auto t3 = std::chrono::high_resolution_clock::now();

        double fw_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        double mps_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

        std::println("  n={}: Floyd-Warshall={:.1f}ms, Min-plus-sq={:.1f}ms, match={}",
                     n, fw_ms, mps_ms, verify_apsp(fw, mps) ? "YES" : "NO");
    }

    // Test 3: Boolean matrix multiplication approach (Seidel's algorithm)
    std::println("\nTest 3: Unweighted graph (Seidel's boolean approach)");
    {
        constexpr int n = 20;
        auto adj = random_unweighted_graph(n, 0.3);

        // Ground truth via BFS
        Matrix bfs_dist(n, std::vector<double>(n, INF));
        for (int s : std::views::iota(0, n)) {
            bfs_dist[s][s] = 0;
            std::vector<bool> visited(n, false);
            visited[s] = true;
            std::queue<int> q;
            q.push(s);
            while (!q.empty()) {
                int u = q.front(); q.pop();
                for (int v : std::views::iota(0, n))
                    if (adj[u][v] && !visited[v]) {
                        visited[v] = true;
                        bfs_dist[s][v] = bfs_dist[s][u] + 1;
                        q.push(v);
                    }
            }
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        auto seidel = seidel_apsp_unweighted(adj);
        auto t2 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

        std::println("  n={}: Seidel time={:.1f}ms, matches BFS: {}",
                     n, ms, verify_apsp(seidel, bfs_dist) ? "YES" : "NO");
    }
}

} // namespace randalgo
