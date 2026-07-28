#include "ral/game_tree.h"
#include <iostream>

int main() {
    using namespace ral;

    // Build a small game tree
    GameTree tree(0); // root is MAX

    // Level 1: MIN nodes
    int n1 = tree.add_node(NodeType::MIN_NODE);
    int n2 = tree.add_node(NodeType::MIN_NODE);
    tree.add_child(0, n1);
    tree.add_child(0, n2);

    // Level 2: leaves
    tree.add_child(n1, tree.add_node(NodeType::LEAF, 0.3));
    tree.add_child(n1, tree.add_node(NodeType::LEAF, 0.7));
    tree.add_child(n2, tree.add_node(NodeType::LEAF, 0.5));
    tree.add_child(n2, tree.add_node(NodeType::LEAF, 0.9));

    println("=== Game Tree Evaluation ===");
    double det_value = deterministic_evaluate(tree, 0);
    println("Deterministic (minimax): {:.4f}", det_value);

    std::mt19937 rng(42);
    double rand_value = randomized_evaluate(tree, 0, rng);
    println("Randomized (shuffled):  {:.4f}", rand_value);

    // Monte Carlo evaluation
    auto mc = monte_carlo_evaluate(tree, 1000, 42);
    println("Monte Carlo (1000 samples): {:.4f}", mc.estimate);

    // Build a larger tree and compare leaf evaluations
    println("\n=== Binary Tree (depth=8) ===");
    auto big_tree = GameTree::build_binary_tree(8, 42);
    double big_det = deterministic_evaluate(big_tree, 0);
    println("Deterministic value: {:.4f}", big_det);
    println("Leaves evaluated (deterministic): {}",
           count_leaves_evaluated(big_tree, 0));

    std::mt19937 rng2(42);
    double big_rand = randomized_evaluate(big_tree, 0, rng2);
    println("Randomized value: {:.4f}", big_rand);

    return 0;
}
