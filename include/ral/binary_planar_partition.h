#pragma once

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include "random_utils.h"

namespace chapter1 {
using randalgo::rng;

// Line segment in 2D
struct Segment {
    double x1, y1, x2, y2;
    int id;
    
    Segment(double x1, double y1, double x2, double y2, int id = -1) 
        : x1(x1), y1(y1), x2(x2), y2(y2), id(id) {}
    
    // Get the line extending this segment to infinity
    // Returns direction vector (dx, dy) and a point on the line
    std::pair<std::pair<double, double>, std::pair<double, double>> get_line() const {
        double dx = x2 - x1;
        double dy = y2 - y1;
        double len = std::sqrt(dx * dx + dy * dy);
        return {{dx / len, dy / len}, {x1, y1}};
    }
};

// Binary planar partition tree node
struct PartitionNode {
    Segment* partition_line;  // Line that splits this region (nullptr for leaves)
    Segment* contained_segment;  // Segment contained in this region (nullptr for internal nodes)
    PartitionNode* left;
    PartitionNode* right;
    
    PartitionNode() : partition_line(nullptr), contained_segment(nullptr), left(nullptr), right(nullptr) {}
    
    ~PartitionNode() {
        delete left;
        delete right;
    }
};

// Check if two segments intersect
bool segments_intersect(const Segment& s1, const Segment& s2) {
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

// Compute index(u, v): number of segments that l(u) intersects before hitting v
int compute_index(const Segment& u, const Segment& v, const std::vector<Segment>& segments) {
    // Find the line extending u (used for intersection checks)
    auto [dir, point] = u.get_line();
    (void)dir;  // Suppress unused variable warning
    
    int count = 0;
    for (const auto& s : segments) {
        if (s.id == u.id || s.id == v.id) continue;
        
        // Check if the line through u intersects s
        // Simplified: just count intersections
        if (segments_intersect(u, s)) {
            count++;
        }
    }
    return count;
}

// Randomized auto-partition algorithm (RandAuto)
PartitionNode* rand_auto_partition(std::vector<Segment>& segments) {
    if (segments.empty()) return nullptr;
    
    if (segments.size() == 1) {
        PartitionNode* leaf = new PartitionNode();
        leaf->contained_segment = &segments[0];
        return leaf;
    }
    
    // Pick a random permutation of segment indices
    std::vector<int> perm(segments.size());
    std::iota(perm.begin(), perm.end(), 0);
    rng().shuffle(perm);
    
    // Find the first segment in the permutation that can be used as partition
    int partition_idx = perm[0];
    
    PartitionNode* node = new PartitionNode();
    node->partition_line = &segments[partition_idx];
    
    // Split segments into those intersected by the partition line and those not
    std::vector<Segment> left_segments, right_segments;
    
    for (size_t i = 0; i < segments.size(); i++) {
        if (static_cast<int>(i) == partition_idx) continue;
        
        if (segments_intersect(segments[partition_idx], segments[i])) {
            // This segment is intersected by the partition line
            // For simplicity, we'll keep it in both sides (will be split further)
            left_segments.push_back(segments[i]);
            right_segments.push_back(segments[i]);
        } else {
            // Determine which side of the partition line this segment is on
            // Using cross product
            double dx = segments[partition_idx].x2 - segments[partition_idx].x1;
            double dy = segments[partition_idx].y2 - segments[partition_idx].y1;
            double px = segments[i].x1 - segments[partition_idx].x1;
            double py = segments[i].y1 - segments[partition_idx].y1;
            
            double cross = dx * py - dy * px;
            
            if (cross >= 0) {
                left_segments.push_back(segments[i]);
            } else {
                right_segments.push_back(segments[i]);
            }
        }
    }
    
    node->left = rand_auto_partition(left_segments);
    node->right = rand_auto_partition(right_segments);
    
    return node;
}

// Count the size of the partition tree
int partition_size(const PartitionNode* node) {
    if (node == nullptr) return 0;
    return 1 + partition_size(node->left) + partition_size(node->right);
}

// Count the number of segments that are split (appear in multiple leaves)
int count_splits(const PartitionNode* node, std::vector<bool>& visited) {
    if (node == nullptr) return 0;
    
    int splits = 0;
    
    if (node->contained_segment != nullptr) {
        int id = node->contained_segment->id;
        if (visited[id]) {
            splits++;  // This segment appears again
        }
        visited[id] = true;
    }
    
    splits += count_splits(node->left, visited);
    splits += count_splits(node->right, visited);
    
    return splits;
}

void demonstrate_binary_planar_partitions() {
    std::cout << "=== Binary Planar Partitions ===\n\n";
    
    // Create example segments (from Figure 1.2 in the book)
    std::vector<Segment> segments = {
        Segment(0, 0, 4, 2, 0),    // Segment 1
        Segment(1, 3, 3, 1, 1),    // Segment 2
        Segment(2, 0, 2, 4, 2)     // Segment 3
    };
    
    std::cout << "Input: " << segments.size() << " line segments\n";
    for (const auto& s : segments) {
        std::cout << "  Segment " << s.id << ": (" << s.x1 << "," << s.y1 << ") -> (" 
                  << s.x2 << "," << s.y2 << ")\n";
    }
    
    // Build the partition
    PartitionNode* root = rand_auto_partition(segments);
    
    int size = partition_size(root);
    std::cout << "\nPartition tree size: " << size << "\n";
    
    // Theorem 1.2: Expected size is O(n log n)
    double theoretical_bound = segments.size() * std::log2(segments.size() + 1);
    std::cout << "Theoretical bound O(n log n): " << theoretical_bound << "\n";
    
    // Expected size analysis
    std::cout << "\nAnalysis:\n";
    std::cout << "For n segments, expected partition size is O(n log n)\n";
    std::cout << "This is because for each pair of segments (u, v),\n";
    std::cout << "the probability that l(u) cuts v is at most 1/(index(u,v) + 1)\n";
    std::cout << "Summing over all pairs gives O(n log n) expected intersections\n";
    
    delete root;
}

// Sailor problem from Example 1.1
// 40 sailors return to ship, each chooses random cabin
// What's the expected number of sailors in their own cabin?
void demonstrate_sailor_problem() {
    std::cout << "\n=== Example 1.1: Sailor Problem ===\n\n";
    
    int n_sailors = 40;
    int num_trials = 100000;
    
    long long total_correct = 0;
    
    for (int trial = 0; trial < num_trials; trial++) {
        // Create random permutation of cabins
        std::vector<int> cabin_assignment(n_sailors);
        std::iota(cabin_assignment.begin(), cabin_assignment.end(), 0);
        rng().shuffle(cabin_assignment);
        
        // Count sailors in correct cabin
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