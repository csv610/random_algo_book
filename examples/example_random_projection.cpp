// =============================================================================
// Random Projection Preserving Pairwise Distances
//
// Given a dataset of high-dimensional points, apply a random Gaussian
// projection to a lower-dimensional space while preserving distances
// between ALL pairs of points with high probability.
//
// This is the practical application of the Johnson-Lindenstrauss lemma:
// the projection matrix R ~ N(0, 1/sqrt(k)) ensures that for any two
// points u, v in the original space:
//   ||Ru - Rv||^2 ≈ ||u - v||^2
//
// We measure accuracy via the ratio of projected to original distances
// across the full dataset.
// =============================================================================

#include "ral/random_projection.h"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <numeric>
#include <iomanip>

namespace ral {

inline double mean(const std::vector<double>& v) {
    return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
}

inline double stddev(const std::vector<double>& v) {
    double m = mean(v);
    double sum = 0.0;
    for (double x : v) sum += (x - m) * (x - m);
    return std::sqrt(sum / v.size());
}

inline void demonstrate_random_projection() {
    std::mt19937 rng(123);

    std::cout << "=== Random Projection: Distance Preservation ===\n\n";

    // Create a structured dataset: 200 points on a 50-D manifold
    int n = 200, d = 50;
    std::normal_distribution<double> gauss(0.0, 1.0);

    // Points = random combination of 5 basis directions + noise
    std::vector<std::vector<double>> points(n, std::vector<double>(d, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < 5; ++k) {
            double coeff = gauss(rng);
            for (int j = 0; j < d; ++j) {
                points[i][j] += coeff * gauss(rng);
            }
        }
        for (int j = 0; j < d; ++j) {
            points[i][j] += 0.1 * gauss(rng);  // small noise
        }
    }

    // Test projection to different target dimensions
    std::vector<int> targets = {3, 5, 10, 20, 40};

    std::cout << "Dataset: " << n << " points in R^" << d << "\n";
    std::cout << "Structure: 5 latent factors + noise\n\n";

    std::cout << std::left
              << std::setw(10) << "Target k"
              << std::setw(16) << "Mean ratio"
              << std::setw(16) << "Std ratio"
              << std::setw(16) << "Min ratio"
              << std::setw(16) << "Max ratio" << "\n";
    std::cout << std::string(74, '-') << "\n";

    for (int k : targets) {
        RandomProjection rp(d, k, rng);
        auto proj = rp.project_dataset(points);

        std::vector<double> ratios;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double d_orig = std::sqrt(RandomProjection::squared_distance(points[i], points[j]));
                double d_proj = std::sqrt(RandomProjection::squared_distance(proj[i], proj[j]));
                if (d_orig > 1e-10) {
                    ratios.push_back(d_proj / d_orig);
                }
            }
        }

        double mn = *std::min_element(ratios.begin(), ratios.end());
        double mx = *std::max_element(ratios.begin(), ratios.end());

        std::cout << std::setw(10) << k
                  << std::setw(16) << std::fixed << std::setprecision(4) << mean(ratios)
                  << std::setw(16) << std::fixed << std::setprecision(4) << stddev(ratios)
                  << std::setw(16) << std::fixed << std::setprecision(4) << mn
                  << std::setw(16) << std::fixed << std::setprecision(4) << mx << "\n";
    }

    // Nearest neighbor preservation test
    std::cout << "\n--- Nearest Neighbor Preservation (k=10) ---\n";
    int target_k = 10;
    RandomProjection rp(d, target_k, rng);
    auto proj = rp.project_dataset(points);

    int preserved = 0;
    int total = std::min(50, n);
    for (int i = 0; i < total; ++i) {
        // Find nearest neighbor in original space
        double best_orig = 1e18;
        int nn_orig = -1;
        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            double dist = RandomProjection::squared_distance(points[i], points[j]);
            if (dist < best_orig) { best_orig = dist; nn_orig = j; }
        }

        // Find nearest neighbor in projected space
        double best_proj = 1e18;
        int nn_proj = -1;
        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            double dist = RandomProjection::squared_distance(proj[i], proj[j]);
            if (dist < best_proj) { best_proj = dist; nn_proj = j; }
        }

        if (nn_orig == nn_proj) preserved++;
    }
    std::cout << "  NN preserved: " << preserved << "/" << total
              << " (" << 100.0 * preserved / total << "%)\n";

    // Show one specific pair
    int i = 0, j = 1;
    double d_orig = std::sqrt(RandomProjection::squared_distance(points[i], points[j]));
    double d_proj = std::sqrt(RandomProjection::squared_distance(proj[i], proj[j]));
    std::cout << "  Pair (0,1): original dist = " << std::fixed << std::setprecision(4) << d_orig
              << ", projected dist = " << d_proj
              << ", ratio = " << d_proj / d_orig << "\n";
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_random_projection();
    return 0;
}
