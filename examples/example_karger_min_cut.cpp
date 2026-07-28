/*
 * Karger's Randomized Minimum Cut Algorithm
 *
 * Repeatedly contracts a randomly chosen edge until only 2 vertices remain.
 * The remaining edges constitute a cut. Running many times and taking the
 * minimum gives the min-cut with high probability.
 *
 * Each single run succeeds with probability >= 2/(n*(n-1)).
 * After n^2/2 repetitions the failure probability drops below 1/e.
 *
 * Reference: "Global minimum cuts in O(n^2) time" - David Karger, 1993
 */

#include "ral/min_cut.h"
#include <iostream>
#include <cmath>

using namespace ral;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Karger's Randomized Min-Cut Algorithm\n";
    std::cout << "========================================================\n\n";

    // Build the graph from Chapter 1: 6 vertices
    Multigraph G(6);
    G.add_edge(0, 1); G.add_edge(0, 2); G.add_edge(0, 3);
    G.add_edge(1, 2); G.add_edge(1, 4);
    G.add_edge(2, 3); G.add_edge(2, 5);
    G.add_edge(3, 4); G.add_edge(3, 5);
    G.add_edge(4, 5);

    int n = G.n;
    std::cout << "Graph: " << n << " vertices, " << G.total_edges() << " edges\n\n";

    // 1) Single random contraction
    int single = karger_min_cut_size(G);
    std::cout << "Single Karger run:      " << single << "\n";

    // 2) Repeated Karger: n^2/2 trials guarantee ~1/e failure probability
    int trials = n * n / 2;
    int repeated = karger_repeated(G, trials);
    std::cout << "Karger repeated " << trials << " times: " << repeated << "\n";

    // 3) Exact answer via max-flow for verification
    int exact = exact_min_cut(G, n);
    std::cout << "Exact min-cut (max-flow): " << exact << "\n";

    // 4) Success probability analysis
    double prob = 2.0 / (n * (n - 1));
    double fail_prob = std::pow(1.0 - prob, trials);
    std::cout << "\nTheoretical P(success per run): " << prob << "\n";
    std::cout << "P(failure all " << trials << " runs): " << fail_prob << "\n";
    std::cout << "Match exact: " << (repeated == exact ? "YES" : "NO") << "\n";

    return 0;
}
