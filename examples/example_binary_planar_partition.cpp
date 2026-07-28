// =============================================================================
// Randomized Binary Planar Partition (RandAuto)
// =============================================================================
//
// Partitions the plane with respect to a set of n line segments into regions,
// each containing at most one segment. Uses a randomized algorithm that
// recursively picks a random segment as a "splitter" and divides the
// remaining segments by which side of the splitter they lie on.
//
// Segments that cross the splitter are duplicated into both sides.
//
// Expected partition size: O(n log n), achieved by randomizing the order
// of segment processing. For each pair (u,v), the probability that the
// line containing u cuts v is at most 1/(rank of u in the permutation).
// Summing over all pairs yields O(n log n) expected total splits.
//
// This is used as a subroutine in larger geometric algorithms, e.g.,
// for triangulating monotone subdivisions.
// =============================================================================

#include <iostream>
#include <vector>
#include <memory>
#include <iomanip>
#include <cmath>

#include "ral/binary_planar_partition.h"

using namespace ral;

int main() {
    std::cout << "=== Randomized Binary Planar Partition ===\n\n";

    // --- Demo 1: Small example with 3 segments ---
    std::cout << "Demo 1: Three line segments\n";
    {
        std::vector<Segment> segments = {
            Segment(0, 0, 4, 2, 0),   // diagonal
            Segment(1, 3, 3, 1, 1),   // crossing diagonal
            Segment(2, 0, 2, 4, 2)    // vertical
        };

        std::cout << "  Segments:\n";
        for (const auto& s : segments) {
            std::cout << "    [" << s.id << "] ("
                      << s.x1 << "," << s.y1 << ") -> ("
                      << s.x2 << "," << s.y2 << ")\n";
        }

        std::unique_ptr<PartitionNode> root(rand_auto_partition(segments));
        int size = partition_size(root.get());

        std::cout << "  Partition tree size: " << size << "\n";
        std::cout << "  Theoretical O(n log n) bound: "
                  << segments.size() * std::log2(segments.size() + 1) << "\n\n";
    }

    // --- Demo 2: Larger random set of segments ---
    std::cout << "Demo 2: Random non-crossing segments\n";
    {
        std::mt19937 rng(42);
        std::uniform_real_distribution<double> dist(0.0, 100.0);

        int n = 20;
        std::vector<Segment> segments;
        for (int i = 0; i < n; i++) {
            double x1 = dist(rng), y1 = dist(rng);
            double x2 = dist(rng), y2 = dist(rng);
            segments.push_back(Segment(x1, y1, x2, y2, i));
        }

        std::unique_ptr<PartitionNode> root(rand_auto_partition(segments));
        int size = partition_size(root.get());

        std::cout << "  Segments: " << n << "\n";
        std::cout << "  Partition tree size: " << size << "\n";
        std::cout << "  O(n log n) bound: " << n * std::log2(n + 1) << "\n\n";
    }

    // --- Demo 3: Split analysis ---
    std::cout << "Demo 3: Split count analysis (how many segments are duplicated)\n";
    {
        int n = 30;
        std::vector<Segment> segments;

        // Create mostly non-crossing horizontal segments at different y-values
        for (int i = 0; i < n; i++) {
            double y = i * 3.0;
            segments.push_back(Segment(0, y, 100, y + (i % 3) * 2, i));
        }

        std::unique_ptr<PartitionNode> root(rand_auto_partition(segments));
        std::vector<bool> visited(n, false);
        int splits = count_splits(root.get(), visited);

        std::cout << "  Segments: " << n << "\n";
        std::cout << "  Segments split (appear in multiple leaves): " << splits << "\n";
        std::cout << "  Partition tree size: " << partition_size(root.get()) << "\n\n";
    }

    // --- Demo 4: Scaling ---
    std::cout << "Demo 4: Scaling — partition tree size vs n\n";
    std::cout << std::setw(10) << "  n"
              << std::setw(14) << "tree_size"
              << std::setw(14) << "n*log2(n)" << "\n";
    std::cout << "  " << std::string(38, '-') << "\n";

    for (int n : {5, 10, 20, 50, 100}) {
        std::vector<Segment> segments;
        for (int i = 0; i < n; i++) {
            double x1 = i * 2.0, y1 = i * 1.5;
            double x2 = x1 + 5.0, y2 = y1 + 3.0;
            segments.push_back(Segment(x1, y1, x2, y2, i));
        }

        std::unique_ptr<PartitionNode> root(rand_auto_partition(segments));
        int size = partition_size(root.get());

        std::cout << std::setw(10) << n
                  << std::setw(14) << size
                  << std::setw(14) << std::fixed << std::setprecision(1)
                  << n * std::log2(n + 1) << "\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}
