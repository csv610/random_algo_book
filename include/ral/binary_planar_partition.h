#pragma once

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory>
#include "random_utils.h"

namespace ral {

// Line segment in 2D
struct Segment {
    double x1, y1, x2, y2;
    int id;

    Segment(double x1, double y1, double x2, double y2, int id = -1)
        : x1(x1), y1(y1), x2(x2), y2(y2), id(id) {}
};

// Binary planar partition tree node
// Stores indices into the original segment vector to avoid dangling pointers.
struct PartitionNode {
    int partition_idx;     // Index of segment that splits this region (-1 for leaves)
    int contained_idx;     // Index of segment contained in this region (-1 for internal)
    PartitionNode* left;
    PartitionNode* right;

    PartitionNode() : partition_idx(-1), contained_idx(-1), left(nullptr), right(nullptr) {}

    ~PartitionNode() {
        delete left;
        delete right;
    }

    PartitionNode(const PartitionNode&) = delete;
    PartitionNode& operator=(const PartitionNode&) = delete;
};

// Check if two segments intersect
inline bool segments_intersect(const Segment& s1, const Segment& s2) {
    auto orientation = [](double px, double py, double qx, double qy, double rx, double ry) {
        double val = (qy - py) * (rx - qx) - (qx - px) * (ry - qy);
        if (std::abs(val) < 1e-10) return 0;
        return (val > 0) ? 1 : 2;
    };

    auto on_segment = [](double px, double py, double qx, double qy, double rx, double ry) {
        if (std::min(px, qx) <= rx && rx <= std::max(px, qx) &&
            std::min(py, qy) <= ry && ry <= std::max(py, qy))
            return true;
        return false;
    };

    int o1 = orientation(s1.x1, s1.y1, s1.x2, s1.y2, s2.x1, s2.y1);
    int o2 = orientation(s1.x1, s1.y1, s1.x2, s1.y2, s2.x2, s2.y2);
    int o3 = orientation(s2.x1, s2.y1, s2.x2, s2.y2, s1.x1, s1.y1);
    int o4 = orientation(s2.x1, s2.y1, s2.x2, s2.y2, s1.x2, s1.y2);

    if (o1 != o2 && o3 != o4) return true;

    if (o1 == 0 && on_segment(s1.x1, s1.y1, s1.x2, s1.y2, s2.x1, s2.y1)) return true;
    if (o2 == 0 && on_segment(s1.x1, s1.y1, s1.x2, s1.y2, s2.x2, s2.y2)) return true;
    if (o3 == 0 && on_segment(s2.x1, s2.y1, s2.x2, s2.y2, s1.x1, s1.y1)) return true;
    if (o4 == 0 && on_segment(s2.x1, s2.y1, s2.x2, s2.y2, s1.x2, s1.y2)) return true;

    return false;
}

namespace detail {

inline PartitionNode* rand_auto_partition_impl(
    const std::vector<Segment>& segments,
    const std::vector<int>& indices) {

    if (indices.empty()) return nullptr;

    if (indices.size() == 1) {
        auto* leaf = new PartitionNode();
        leaf->contained_idx = indices[0];
        return leaf;
    }

    // Pick a random permutation of indices
    std::vector<int> perm = indices;
    rng().shuffle(perm);

    int partition_idx = perm[0];

    auto* node = new PartitionNode();
    node->partition_idx = partition_idx;

    // Split remaining indices by side of the partition line
    std::vector<int> left_indices, right_indices;

    for (size_t i = 1; i < perm.size(); ++i) {
        int idx = perm[i];

        if (segments_intersect(segments[partition_idx], segments[idx])) {
            left_indices.push_back(idx);
            right_indices.push_back(idx);
        } else {
            double dx = segments[partition_idx].x2 - segments[partition_idx].x1;
            double dy = segments[partition_idx].y2 - segments[partition_idx].y1;
            double px = segments[idx].x1 - segments[partition_idx].x1;
            double py = segments[idx].y1 - segments[partition_idx].y1;

            double cross = dx * py - dy * px;

            if (cross >= 0) {
                left_indices.push_back(idx);
            } else {
                right_indices.push_back(idx);
            }
        }
    }

    node->left = rand_auto_partition_impl(segments, left_indices);
    node->right = rand_auto_partition_impl(segments, right_indices);

    return node;
}

} // namespace detail

// Randomized auto-partition algorithm (RandAuto)
inline PartitionNode* rand_auto_partition(const std::vector<Segment>& segments) {
    std::vector<int> indices(segments.size());
    std::iota(indices.begin(), indices.end(), 0);
    return detail::rand_auto_partition_impl(segments, indices);
}

// Count the size of the partition tree
inline int partition_size(const PartitionNode* node) {
    if (node == nullptr) return 0;
    return 1 + partition_size(node->left) + partition_size(node->right);
}

// Count the number of segments that are split (appear in multiple leaves)
inline int count_splits(const PartitionNode* node, std::vector<bool>& visited) {
    if (node == nullptr) return 0;

    int splits = 0;

    if (node->contained_idx >= 0) {
        int id = node->contained_idx;
        if (visited[id]) {
            splits++;
        }
        visited[id] = true;
    }

    splits += count_splits(node->left, visited);
    splits += count_splits(node->right, visited);

    return splits;
}

inline void demonstrate_binary_planar_partitions() {
    std::cout << "=== Binary Planar Partitions ===\n\n";

    std::vector<Segment> segments = {
        Segment(0, 0, 4, 2, 0),
        Segment(1, 3, 3, 1, 1),
        Segment(2, 0, 2, 4, 2)
    };

    std::cout << "Input: " << segments.size() << " line segments\n";
    for (const auto& s : segments) {
        std::cout << "  Segment " << s.id << ": (" << s.x1 << "," << s.y1 << ") -> ("
                  << s.x2 << "," << s.y2 << ")\n";
    }

    std::unique_ptr<PartitionNode> root(rand_auto_partition(segments));

    int size = partition_size(root.get());
    std::cout << "\nPartition tree size: " << size << "\n";

    double theoretical_bound = segments.size() * std::log2(segments.size() + 1);
    std::cout << "Theoretical bound O(n log n): " << theoretical_bound << "\n";

    std::cout << "\nAnalysis:\n";
    std::cout << "For n segments, expected partition size is O(n log n)\n";
    std::cout << "This is because for each pair of segments (u, v),\n";
    std::cout << "the probability that l(u) cuts v is at most 1/(index(u,v) + 1)\n";
    std::cout << "Summing over all pairs gives O(n log n) expected intersections\n";
}

// Sailor problem from Example 1.1
inline void demonstrate_sailor_problem() {
    std::cout << "\n=== Example 1.1: Sailor Problem ===\n\n";

    int n_sailors = 40;
    int num_trials = 100000;

    long long total_correct = 0;

    for (int trial = 0; trial < num_trials; trial++) {
        std::vector<int> cabin_assignment(n_sailors);
        std::iota(cabin_assignment.begin(), cabin_assignment.end(), 0);
        rng().shuffle(cabin_assignment);

        int correct = 0;
        for (int i = 0; i < n_sailors; i++) {
            if (cabin_assignment[i] == i) {
                correct++;
            }
        }

        total_correct += correct;
    }

    double expected_correct = static_cast<double>(total_correct) / num_trials;

    std::cout << "Problem: " << n_sailors << " sailors, each chooses random cabin\n";
    std::cout << "Expected number in own cabin (by linearity of expectation): 1\n";
    std::cout << "Empirical average over " << num_trials << " trials: " << expected_correct << "\n\n";

    std::cout << "Proof using indicator variables:\n";
    std::cout << "Let X_i = 1 if sailor i is in own cabin, 0 otherwise\n";
    std::cout << "E[X_i] = 1/n for each i\n";
    std::cout << "E[sum X_i] = sum E[X_i] = n * (1/n) = 1\n\n";
}

} // namespace chapter1
