// =============================================================================
// Johnson-Lindenstrauss Dimension Reduction
//
// The JL lemma states: for any set of n points in R^d and target dimension k,
// there exists a linear map f: R^d -> R^k such that for all pairs (u,v):
//   (1 - eps) ||u-v||^2 <= ||f(u)-f(v)||^2 <= (1 + eps) ||u-v||^2
// with high probability, when k >= O(eps^{-2} log n).
//
// A random Gaussian projection matrix R (k x d) scaled by 1/sqrt(k)
// achieves this: project each point x as (R*x).
//
// This example shows distance preservation between random point pairs.
// =============================================================================

#include "ral/random_projection.h"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <iomanip>

namespace ral {

inline void demonstrate_jl_projection() {
    std::mt19937 rng(42);

    std::cout << "=== Johnson-Lindenstrauss Dimension Reduction ===\n\n";

    int original_dim = 100;
    int num_points = 50;
    double eps = 0.25;  // 25% distortion allowed

    // JL bound: k >= (8/eps^2) * ln(n) for (1+eps)-distortion with high prob
    int min_k = static_cast<int>(std::ceil(8.0 / (eps * eps) * std::log(num_points)));
    std::cout << "Parameters:\n";
    std::cout << "  Original dimension d = " << original_dim << "\n";
    std::cout << "  Number of points n   = " << num_points << "\n";
    std::cout << "  Target distortion eps= " << eps << "\n";
    std::cout << "  JL bound: k >= " << min_k << "\n\n";

    // Generate random points in R^d
    std::normal_distribution<double> gauss(0.0, 1.0);
    std::vector<std::vector<double>> points(num_points, std::vector<double>(original_dim));
    for (int i = 0; i < num_points; ++i)
        for (int j = 0; j < original_dim; ++j)
            points[i][j] = gauss(rng);

    // Test multiple target dimensions
    std::vector<int> target_dims = {2, 5, min_k, 30, 50};
    std::cout << std::left << std::setw(12) << "Target k"
              << std::setw(18) << "Max stretch"
              << std::setw(18) << "Max shrink"
              << std::setw(12) << "Within eps?" << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (int k : target_dims) {
        RandomProjection rp(original_dim, k, rng);

        // Project all points
        auto projected = rp.project_dataset(points);

        // Compute pairwise distance ratios
        double max_stretch = 0.0;
        double max_shrink = 0.0;
        int num_pairs = 0;
        int violations = 0;

        for (int i = 0; i < num_points; ++i) {
            for (int j = i + 1; j < num_points; ++j) {
                double orig_dist2 = RandomProjection::squared_distance(points[i], points[j]);
                double proj_dist2 = RandomProjection::squared_distance(projected[i], projected[j]);

                if (orig_dist2 < 1e-10) continue;

                double ratio = proj_dist2 / orig_dist2;
                max_stretch = std::max(max_stretch, ratio);
                max_shrink = std::max(max_shrink, 1.0 / ratio);
                num_pairs++;

                if (ratio > 1.0 + eps || ratio < 1.0 - eps) {
                    violations++;
                }
            }
        }

        bool within = (max_stretch <= 1.0 + eps) && (max_shrink <= 1.0 + eps);
        std::cout << std::setw(12) << k
                  << std::setw(18) << std::fixed << std::setprecision(4) << max_stretch
                  << std::setw(18) << std::fixed << std::setprecision(4) << max_shrink
                  << std::setw(12) << (within ? "YES" : "NO")
                  << "  (" << violations << "/" << num_pairs << " violations)\n";
    }

    // Practical example: 1000-D data -> 10-D
    std::cout << "\n--- Practical Example: 1000D -> 10D ---\n";
    int big_d = 1000, small_k = 10, pts = 100;
    std::vector<std::vector<double>> big_points(pts, std::vector<double>(big_d));
    for (int i = 0; i < pts; ++i)
        for (int j = 0; j < big_d; ++j)
            big_points[i][j] = gauss(rng);

    RandomProjection rp_big(big_d, small_k, rng);
    auto proj_big = rp_big.project_dataset(big_points);

    // Check a few distances
    double total_ratio = 0.0;
    int checked = 0;
    for (int i = 0; i < std::min(20, pts); ++i) {
        for (int j = i + 1; j < std::min(20, pts); ++j) {
            double d_orig = std::sqrt(RandomProjection::squared_distance(big_points[i], big_points[j]));
            double d_proj = std::sqrt(RandomProjection::squared_distance(proj_big[i], proj_big[j]));
            total_ratio += d_proj / d_orig;
            checked++;
        }
    }
    std::cout << "  Average distance ratio (projected/original): "
              << std::fixed << std::setprecision(4) << total_ratio / checked << "\n";
    std::cout << "  (ideal = 1.0,  compression ratio: " << big_d << " -> " << small_k
              << " = " << std::fixed << std::setprecision(1) << 100.0 * small_k / big_d << "%)\n";
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_jl_projection();
    return 0;
}
