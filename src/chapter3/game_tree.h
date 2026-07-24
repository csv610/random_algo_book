#pragma once
// Game Tree Evaluation: Randomized evaluation of AND-OR trees
// Theorem 2.1: Expected cost is at most 3^k for T_{2,k} with n=4^k leaves

#include <vector>
#include <random>
#include <memory>
#include <iostream>
#include <chrono>

namespace chapter3 {

enum class NodeType { AND, OR };

struct TreeNode {
    NodeType type;
    bool is_leaf;
    int value;
    std::vector<std::unique_ptr<TreeNode>> children;

    TreeNode(NodeType t) : type(t), is_leaf(false), value(0) {}
    TreeNode(int v) : type(NodeType::AND), is_leaf(true), value(v) {}
};

struct EvalResult {
    bool val;
    int leaves_inspected;
};

class GameTreeEvaluator {
public:
    GameTreeEvaluator(unsigned seed = std::chrono::steady_clock::now()
                              .time_since_epoch().count())
        : rng_(seed) {}

    // Build a complete binary AND-OR tree of depth 2k
    // Root is AND (even depth), children at odd depth are OR, etc.
    std::unique_ptr<TreeNode> build_tree(int k) {
        if (k == 0) {
            return std::make_unique<TreeNode>(
                std::uniform_int_distribution<>(0, 1)(rng_));
        }
        NodeType type = (k % 2 == 1) ? NodeType::AND : NodeType::OR;
        auto node = std::make_unique<TreeNode>(type);
        node->children.push_back(build_tree(k - 1));
        node->children.push_back(build_tree(k - 1));
        return node;
    }

    // Randomized evaluation of AND node
    EvalResult eval_and(TreeNode* v) {
        if (v->is_leaf) return {v->value == 1, 1};

        int n = v->children.size();
        int pick = std::uniform_int_distribution<>(0, n - 1)(rng_);

        auto r = eval_or(v->children[pick].get());
        int cost = r.leaves_inspected;
        if (!r.val) return {false, cost};

        // Must check other children
        for (int i = 0; i < n; ++i) {
            if (i == pick) continue;
            auto r2 = eval_or(v->children[i].get());
            cost += r2.leaves_inspected;
            if (!r2.val) return {false, cost};
        }
        return {true, cost};
    }

    // Randomized evaluation of OR node
    EvalResult eval_or(TreeNode* v) {
        if (v->is_leaf) return {v->value == 1, 1};

        int n = v->children.size();
        int pick = std::uniform_int_distribution<>(0, n - 1)(rng_);

        auto r = eval_and(v->children[pick].get());
        int cost = r.leaves_inspected;
        if (r.val) return {true, cost};

        for (int i = 0; i < n; ++i) {
            if (i == pick) continue;
            auto r2 = eval_and(v->children[i].get());
            cost += r2.leaves_inspected;
            if (r2.val) return {true, cost};
        }
        return {false, cost};
    }

    // Evaluate root (always AND for T_{2,k})
    EvalResult evaluate(TreeNode* root) {
        return eval_and(root);
    }

private:
    std::mt19937 rng_;
};

void demonstrate_game_tree() {
    std::cout << "Randomized Game Tree Evaluation (Theorem 2.1)\n";
    std::cout << "Expected cost: O(n^0.793) vs deterministic O(n)\n\n";

    GameTreeEvaluator evaluator;

    for (int k = 1; k <= 7; ++k) {
        int n = 1 << (2 * k);  // 4^k leaves
        double total = 0;
        int trials = 5000;

        for (int t = 0; t < trials; ++t) {
            auto root = evaluator.build_tree(k);
            auto r = evaluator.evaluate(root.get());
            total += r.leaves_inspected;
        }

        double avg = total / trials;
        double bound = std::pow(3, k);
        double theory = std::pow(n, 0.79248);

        std::cout << "k=" << k
                  << "  n=" << n
                  << "  avg=" << avg
                  << "  3^k=" << bound
                  << "  n^0.793=" << theory
                  << "  ratio=" << (avg / theory) << "\n";
    }
    std::cout << "\n";
}

}  // namespace chapter3
