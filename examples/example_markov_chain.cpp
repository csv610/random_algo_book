#include "ral/markov_chain.h"
#include <iostream>

int main() {
    using namespace ral;

    // Simple 4-state Markov chain
    MarkovChain chain(4);
    chain.set_transition(0, 0, 0.3); chain.set_transition(0, 1, 0.7);
    chain.set_transition(1, 0, 0.2); chain.set_transition(1, 1, 0.3);
    chain.set_transition(1, 2, 0.5);
    chain.set_transition(2, 2, 0.4); chain.set_transition(2, 3, 0.6);
    chain.set_transition(3, 3, 0.8); chain.set_transition(3, 0, 0.2);

    println("=== Markov Chain Analysis ===");
    println("Valid: {}", chain.valid() ? "YES" : "NO");

    auto pi = chain.stationary_distribution();
    println("Stationary distribution:");
    for (int i = 0; i < 4; ++i) {
        println("  state {}: {:.4f}", i, pi[i]);
    }

    // Mixing time
    int mt = chain.mixing_time(0.01);
    println("Mixing time (epsilon=0.01): {} steps", mt);

    // Coupling
    println("\n=== Coupling ===");
    auto coup = coupling_simulation(chain, 0, 3, 200, 42);
    println("Meeting time: {} steps", coup.meeting_time);

    // TV distance over time
    println("\n=== TV Distance ===");
    std::vector<double> init = {1.0, 0.0, 0.0, 0.0};
    for (int t : {1, 5, 10, 20, 50, 100}) {
        double tv = chain.tv_distance(init, t);
        println("  t={}: TV distance = {:.6f}", t, tv);
    }

    // MCMC
    println("\n=== Metropolis-Hastings ===");
    std::vector<double> target = {1.0, 2.0, 3.0, 2.0, 1.0};
    std::vector<std::vector<double>> proposal(5, std::vector<double>(5, 0.2));
    auto samples = metropolis_hastings(target, proposal, 0, 5000, 1000, 42);

    // Count visits to each state
    std::vector<int> counts(5, 0);
    for (int s : samples) counts[s]++;
    println("Sample distribution (should approximate target):");
    double total = 0;
    for (double p : target) total += p;
    for (int i = 0; i < 5; ++i) {
        println("  state {}: count={}, empirical={:.4f}, target={:.4f}",
                i, counts[i], static_cast<double>(counts[i]) / samples.size(),
                target[i] / total);
    }

    // Spanning trees
    println("\n=== Spanning Tree Count (Kirchhoff) ===");
    std::vector<std::pair<int,int>> edges = {{0,1},{0,2},{1,2},{1,3},{2,3}};
    double num_trees = spanning_tree_count_kirchhoff(4, edges);
    println("K4 spanning trees: {:.0f}", num_trees);

    return 0;
}
