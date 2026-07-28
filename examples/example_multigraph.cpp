/*
 * Multigraph Operations
 *
 * A multigraph allows multiple edges between the same pair of vertices.
 * This data structure is fundamental to Karger's min-cut algorithm,
 * where edge contractions naturally create multi-edges.
 *
 * Operations demonstrated:
 *   - add_edge: add an edge between two vertices (increments multiplicity)
 *   - get_mult: query the multiplicity of an edge
 *   - all_edges: list all edges (expanding multiplicities)
 *   - total_edges: count total edges
 *   - contract_edge: merge two vertices, combining their edge sets
 */

#include "ral/min_cut.h"
#include <iostream>
#include <iomanip>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Multigraph Operations\n";
    std::cout << "========================================================\n\n";

    // Build a small multigraph
    Multigraph G(4);
    G.add_edge(0, 1);         // single edge 0-1
    G.add_edge(0, 1);         // now double edge 0-1
    G.add_edge(0, 2);         // single edge 0-2
    G.add_edge(1, 2);         // single edge 1-2
    G.add_edge(1, 3);         // single edge 1-3
    G.add_edge(2, 3);         // single edge 2-3
    G.add_edge(2, 3);         // double edge 2-3

    std::cout << "Initial multigraph (4 vertices):\n";
    std::cout << "  Total edges: " << G.total_edges() << "\n";
    std::cout << "  Multiplicity(0,1): " << G.get_mult(0, 1) << "\n";
    std::cout << "  Multiplicity(2,3): " << G.get_mult(2, 3) << "\n";
    std::cout << "  Multiplicity(0,3): " << G.get_mult(0, 3) << "\n";

    std::cout << "\n  All edges (expanded):\n  ";
    auto edges = G.all_edges();
    for (auto [u, v] : edges)
        std::cout << "(" << u << "," << v << ") ";
    std::cout << "\n";

    // Contract edge (0,1): merge vertex 1 into vertex 0
    std::cout << "\n--- After contracting edge (0,1) ---\n";
    contract_edge(G, 0, 1);

    std::cout << "  Vertices remaining: " << G.n << "\n";
    std::cout << "  Total edges: " << G.total_edges() << "\n";
    std::cout << "  Multiplicity(0,2): " << G.get_mult(0, 2) << "\n";
    std::cout << "  Multiplicity(0,3): " << G.get_mult(0, 3) << "\n";

    std::cout << "\n  All edges (expanded):\n  ";
    edges = G.all_edges();
    for (auto [u, v] : edges)
        std::cout << "(" << u << "," << v << ") ";
    std::cout << "\n";

    // Contract another edge
    std::cout << "\n--- After contracting edge (0,2) ---\n";
    contract_edge(G, 0, 2);

    std::cout << "  Vertices remaining: " << G.n << "\n";
    std::cout << "  Total edges: " << G.total_edges() << "\n";
    std::cout << "  These " << G.total_edges() << " edges form the min-cut!\n";

    // Demonstrate building a multigraph from scratch for min-cut
    std::cout << "\n--- Building a multigraph for min-cut testing ---\n";
    Multigraph H(5);
    H.add_edge(0, 1); H.add_edge(0, 2); H.add_edge(0, 3);
    H.add_edge(1, 2); H.add_edge(1, 3);
    H.add_edge(2, 3); H.add_edge(2, 4); H.add_edge(3, 4);

    std::cout << "  Graph: " << H.n << " vertices, " << H.total_edges() << " edges\n";
    int cut = karger_min_cut_size(H);
    std::cout << "  Karger min-cut (single run): " << cut << "\n";

    return 0;
}
