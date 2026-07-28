#include "ral.h"
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Example: Karger's Min-Cut Algorithm (Section 1.1)\n";
    std::cout << "========================================================\n\n";

    using ral::Multigraph;
    using ral::karger_min_cut_size;
    using ral::karger_repeated;
    using ral::exact_min_cut;

    // Graph from Figure 1.1 in the book (5 vertices)
    Multigraph g(5);
    g.add_edge(0, 1); g.add_edge(0, 2); g.add_edge(0, 3);
    g.add_edge(1, 2); g.add_edge(1, 3);
    g.add_edge(2, 3); g.add_edge(2, 4); g.add_edge(3, 4);

    std::cout << "Graph: 5 vertices, " << g.total_edges() << " edges\n";
    std::cout << "Expected min-cut: 2\n\n";

    std::cout << "Single run: " << karger_min_cut_size(g) << "\n";

    int n = g.n;
    int trials = (n * n) / 2;  // n^2/2 trials as suggested in book
    std::cout << "After " << trials << " trials: " << karger_repeated(g, trials) << "\n\n";

    // Theoretical analysis
    double prob_success = 2.0 / (n * (n - 1));
    double prob_failure = std::pow(1.0 - prob_success, trials);
    std::cout << "Theoretical Analysis:\n";
    std::cout << "  P(success in one trial) >= 2/n^2 = " << prob_success << "\n";
    std::cout << "  P(failure all trials) <= " << prob_failure << "\n";
    std::cout << "  1/e = " << 1.0 / std::exp(1.0) << "\n\n";

    // Exact min-cut via max-flow
    int exact = exact_min_cut(g, n);
    std::cout << "Exact (max-flow): " << exact << "\n";

    return 0;
}