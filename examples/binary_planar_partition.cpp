#include "ral.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Example: Binary Planar Partitions (Chapter 1)\n";
    std::cout << "========================================================\n\n";

    // Sailor problem: 40 sailors, random cabin assignment
    std::cout << "--- Sailor Problem (Example 1.1) ---\n";
    std::cout << "40 sailors return to ship, each picks random cabin.\n";
    std::cout << "Expected sailors in own cabin: 1 (by linearity of expectation)\n";
    std::cout << "E[X] = sum E[X_i] = 40 * (1/40) = 1\n\n";

    // Binary planar partition - RandAuto algorithm
    std::cout << "--- RandAuto Binary Planar Partition ---\n";
    ral::Segment segs[] = {
        {0.0, 0.0, 2.0, 2.0, 1},   // Segment 1
        {2.0, 0.0, 0.0, 2.0, 2},   // Segment 2
        {1.0, 0.0, 1.0, 2.0, 3}    // Segment 3
    };
    std::vector<ral::Segment> segments(segs, segs + 3);

    std::cout << "Input: " << segments.size() << " line segments\n";
    for (auto& s : segments) {
        std::cout << "  Segment " << s.id << ": (" << s.x1 << "," << s.y1 << ") -> (" 
                  << s.x2 << "," << s.y2 << ")\n";
    }

    auto root = ral::rand_auto_partition(segments);
    int size = ral::partition_size(root);
    std::cout << "Partition tree size: " << size << "\n";
    std::cout << "Expected O(n log n) = " << segments.size() * std::log2(segments.size() + 1) << "\n";
    delete root;

    return 0;
}