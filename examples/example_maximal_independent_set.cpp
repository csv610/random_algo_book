// ============================================================
// Example: Maximal Independent Set (Chapter 12)
//
// A Maximal Independent Set (MIS) of a graph is a set of vertices
// such that no two are adjacent, and every other vertex is adjacent
// to at least one member of the set. "Maximal" means you cannot add
// another vertex without breaking independence.
//
// Luby's algorithm computes an MIS in O(log n) parallel rounds:
//   1. Each alive vertex picks a random value.
//   2. A vertex becomes a candidate if its random value is the
//      largest among its alive neighbours.
//   3. Each candidate independently joins the MIS with probability
//      1/(2 * degree).
//   4. MIS members and their neighbours are removed from the alive set.
//
// This example also demonstrates:
//   - Random maximal matching (shuffle all edges, greedily accept).
//   - Greedy vertex coloring.
//
// Compile: g++ -std=c++23 -I../include example_maximal_independent_set.cpp \
//          -o example_maximal_independent_set -pthread
// ============================================================

#include "ral/mis.h"
#include <iostream>
#include <vector>
#include <numeric>

using namespace ral;

int main() {
    std::cout << "=== Maximal Independent Set & Matching ===\n";

    // --- Path graph P5: 0-1-2-3-4 ---
    {
        std::cout << "\n--- Path graph P5 ---\n";
        auto g = make_path(5);
        std::cout << "Vertices: 0..4, edges: 0-1, 1-2, 2-3, 3-4\n";
        auto mis = mis_luby_parallel(g);
        auto mm  = maximal_matching(g);
        std::cout << "MIS valid: "
                  << (verify_independent_set(g, mis) ? "YES" : "NO") << "\n";
        std::cout << "Matching valid: "
                  << (verify_matching(g, mm) ? "YES" : "NO") << "\n";
    }

    // --- Cycle graph C6: 0-1-2-3-4-5-0 ---
    {
        std::cout << "\n--- Cycle graph C6 ---\n";
        auto g = make_cycle(6);
        std::cout << "Vertices: 0..5, edges: (i, (i+1)%6)\n";
        auto mis = mis_luby_parallel(g);
        auto mm  = maximal_matching(g);
        std::cout << "MIS valid: "
                  << (verify_independent_set(g, mis) ? "YES" : "NO") << "\n";
        std::cout << "Matching valid: "
                  << (verify_matching(g, mm) ? "YES" : "NO") << "\n";
    }

    // --- Star graph: center=0, leaves=1,2,3,4 ---
    {
        std::cout << "\n--- Star graph S(0, 4) ---\n";
        auto g = make_star(0, 4);
        std::cout << "Center: 0, Leaves: 1,2,3,4\n";
        auto mis = mis_luby_parallel(g);
        auto mm  = maximal_matching(g);
        std::cout << "MIS valid: "
                  << (verify_independent_set(g, mis) ? "YES" : "NO") << "\n";
        std::cout << "Matching valid: "
                  << (verify_matching(g, mm) ? "YES" : "NO") << "\n";
    }

    // --- Random graph G(12, 0.3) ---
    {
        std::cout << "\n--- Random graph G(12, 0.3) ---\n";
        auto g = make_random(12, 0.3);
        std::cout << "12 vertices, edge probability 0.3, seed=42\n";
        auto mis = mis_luby_parallel(g);
        auto mm  = maximal_matching(g);
        std::cout << "MIS valid: "
                  << (verify_independent_set(g, mis) ? "YES" : "NO") << "\n";
        std::cout << "Matching valid: "
                  << (verify_matching(g, mm) ? "YES" : "NO") << "\n";
    }

    // --- Vertex coloring on P5 ---
    {
        std::cout << "\n--- Greedy Vertex Coloring on P5 ---\n";
        auto g = make_path(5);
        auto colors = vertex_coloring(g);
        std::cout << "Color assignment: ";
        for (int i = 0; i < g.n; ++i)
            std::cout << "v" << i << "=C" << colors[i] << " ";
        std::cout << "\n";
        // Verify proper coloring
        bool proper = true;
        for (int u = 0; u < g.n; ++u)
            for (int v : g.adj[u])
                if (colors[u] == colors[v]) proper = false;
        std::cout << "Proper coloring: " << (proper ? "YES" : "NO") << "\n";
    }

    // --- Practical scenario: network security ---
    std::cout << "\n--- Practical: Non-overlapping security patrol routes ---\n";
    std::cout << "  (MIS selects maximum set of stations with no shared coverage)\n";
    {
        // 8 security stations; edges mean overlapping coverage areas
        auto g = Graph(8);
        g.add_edge(0, 1); g.add_edge(0, 2);
        g.add_edge(1, 3); g.add_edge(2, 3);
        g.add_edge(3, 4); g.add_edge(4, 5);
        g.add_edge(5, 6); g.add_edge(5, 7);
        g.add_edge(6, 7);
        std::cout << "  Stations: 0..7\n";
        std::cout << "  Overlaps: (0,1),(0,2),(1,3),(2,3),(3,4),(4,5),(5,6),(5,7),(6,7)\n";
        auto mis = mis_luby_parallel(g);
        auto verts = mis.vertices();
        std::cout << "  Active patrol stations: ";
        for (size_t i = 0; i < verts.size(); ++i)
            std::cout << (i ? ", " : "") << verts[i];
        std::cout << "\n";
        std::cout << "  Stations covered: " << mis.size()
                  << " out of " << g.n << "\n";
    }

    return 0;
}
