// example_potential_method.cpp
// Demonstrates the potential method for amortized analysis, connecting
// it to martingale theory. The potential method assigns a "potential"
// Phi to each state such that the amortized cost a_i = c_i + Phi_i - Phi_{i-1}.
// If Phi is non-negative and Phi_0 = 0, then sum a_i >= sum c_i.
//
// Connection to martingales: If the potential function is chosen so that
// E[Phi_i | Phi_{i-1}] <= Phi_{i-1}, then -Phi_n is a supermartingale,
// giving concentration bounds on the total cost via Azuma-Hoeffding.
//
// This example analyzes Union-Find with union by rank and path compression.

#include "ral/martingales.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <cmath>
#include <random>

using namespace ral;

// Union-Find with rank (no path compression, for clean analysis)
struct UnionFind {
    std::vector<int> parent, rank;
    int num_components;

    explicit UnionFind(int n) : parent(n), rank(n, 0), num_components(n) {
        std::iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        while (parent[x] != x) x = parent[x];
        return x;
    }

    bool unite(int x, int y) {
        int rx = find(x), ry = find(y);
        if (rx == ry) return false;
        if (rank[rx] < rank[ry]) std::swap(rx, ry);
        parent[ry] = rx;
        if (rank[rx] == rank[ry]) ++rank[rx];
        --num_components;
        return true;
    }
};

// Potential: sum of log(rank) over all roots
// Each union increases the potential by at most O(log n)
double compute_potential(const UnionFind& uf) {
    double phi = 0.0;
    for (int i = 0; i < static_cast<int>(uf.parent.size()); i++) {
        if (uf.parent[i] == i && uf.rank[i] > 0)
            phi += std::log2(static_cast<double>(uf.rank[i]));
    }
    return phi;
}

int main() {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "=== Potential Method Analysis ===\n\n";

    // Part 1: Union-Find amortized cost via potential method
    std::cout << "Union-Find (union by rank, no path compression)\n";
    std::cout << "Potential: Phi = sum of log2(rank(r)) over roots r\n\n";

    int n = 1000;
    UnionFind uf(n);

    std::cout << std::setw(10) << "op"
              << std::setw(14) << "actual_cost"
              << std::setw(14) << "potential"
              << std::setw(14) << "amortized"
              << "\n";
    std::cout << std::string(52, '-') << "\n";

    double total_actual = 0;
    double total_amortized = 0;
    double phi_prev = compute_potential(uf);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, n - 1);

    for (int i = 0; i < 20; i++) {
        int a = dist(rng), b = dist(rng);
        int cost_before = (uf.find(a) != uf.find(b)) ? 2 : 1;
        uf.unite(a, b);
        double phi_now = compute_potential(uf);
        double amortized = cost_before + phi_now - phi_prev;

        total_actual += cost_before;
        total_amortized += amortized;

        std::cout << std::setw(10) << i
                  << std::setw(14) << total_actual
                  << std::setw(14) << phi_now
                  << std::setw(14) << total_amortized
                  << "\n";
        phi_prev = phi_now;
    }

    std::cout << "\n  Total actual cost:    " << total_actual << "\n";
    std::cout << "  Total amortized cost: " << total_amortized << "\n";
    std::cout << "  Final potential:      " << compute_potential(uf) << "\n";
    std::cout << "  Since Phi >= 0 and Phi_0 = 0, sum(a_i) >= sum(c_i)\n";

    // Part 2: Connection to martingales - simulated counter
    std::cout << "\n\nPotential Method as a Martingale\n";
    std::cout << "Counter increments by +X_i where X_i ~ Bernoulli(1/2)\n";
    std::cout << "Potential: Phi_k = -X_k (supermartingale since E[X_k] = 1/2)\n\n";

    std::mt19937 rng2(42);
    std::uniform_int_distribution<int> coin(0, 1);

    int X = 0;
    double Phi = 0.0;
    std::cout << std::setw(6) << "k"
              << std::setw(10) << "X_k"
              << std::setw(10) << "Phi_k"
              << std::setw(14) << "E[Phi_{k+1}|k]"
              << "\n";
    std::cout << std::string(40, '-') << "\n";

    for (int k = 0; k < 10; k++) {
        double E_Phi_next = Phi - 0.5;  // E[X_{k+1}] = 1/2
        std::cout << std::setw(6) << k
                  << std::setw(10) << X
                  << std::setw(10) << Phi
                  << std::setw(14) << E_Phi_next
                  << "\n";
        int step = coin(rng2);
        X += step;
        Phi -= step;
    }
    std::cout << std::setw(6) << 10
              << std::setw(10) << X
              << std::setw(10) << Phi
              << "\n";

    std::cout << "\n  Since E[Phi_{k+1} | state_k] <= Phi_k, -Phi is a supermartingale.\n";
    std::cout << "  This gives concentration of the counter via Azuma-Hoeffding.\n";

    return 0;
}
