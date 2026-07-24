#include <iostream>
#include <vector>
#include <unordered_set>
#include <random>
#include <iomanip>
#include <cmath>
#include "ral.h"

int main() {
    std::cout << "========================================================\n";
    std::cout << "         RAL Modern Randomized Algorithms\n";
    std::cout << "========================================================\n\n";

    std::mt19937 rng(42);

    // ----------------------------------------------------------------
    // 1. MinHash Demonstration
    // ----------------------------------------------------------------
    std::cout << "--- 1. MinHash Jaccard Similarity Estimation ---\n";
    std::unordered_set<int> setA = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::unordered_set<int> setB = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    // Intersection = {6, 7, 8, 9, 10} (size 5)
    // Union = {1..15} (size 15)
    // True Jaccard = 5 / 15 = 0.3333

    ral::MinHash minhash(1000, rng);
    auto sigA = minhash.compute_signature(setA);
    auto sigB = minhash.compute_signature(setB);
    double true_jaccard = 5.0 / 15.0;
    double est_jaccard = ral::MinHash::estimate_similarity(sigA, sigB);

    std::cout << "  True Jaccard Similarity:      " << std::fixed << std::setprecision(4) << true_jaccard << "\n";
    std::cout << "  MinHash Estimated Similarity: " << est_jaccard << " (using 1000 hashes)\n\n";

    // ----------------------------------------------------------------
    // 2. Count-Min Sketch Demonstration
    // ----------------------------------------------------------------
    std::cout << "--- 2. Count-Min Sketch Frequency Estimation ---\n";
    ral::CountMinSketch cms(2000, 5, rng); // width = 2000, depth = 5
    
    // Add stream items
    std::vector<int> stream = {42, 42, 42, 7, 42, 100, 7, 42, 42, 100};
    for (int item : stream) {
        cms.add(item);
    }
    
    std::cout << "  Estimated frequency for 42:  " << cms.estimate(42) << " (True: 6)\n";
    std::cout << "  Estimated frequency for 7:   " << cms.estimate(7) << " (True: 2)\n";
    std::cout << "  Estimated frequency for 100: " << cms.estimate(100) << " (True: 2)\n";
    std::cout << "  Estimated frequency for 999: " << cms.estimate(999) << " (True: 0)\n\n";

    // ----------------------------------------------------------------
    // 3. HyperLogLog Cardinality Estimation
    // ----------------------------------------------------------------
    std::cout << "--- 3. HyperLogLog Cardinality Estimation ---\n";
    ral::HyperLogLog hll(10); // precision b = 10, m = 1024 registers
    
    int true_cardinality = 5000;
    for (int i = 0; i < true_cardinality; ++i) {
        hll.insert(i);
    }
    
    std::cout << "  True Cardinality:      " << true_cardinality << "\n";
    std::cout << "  HyperLogLog Estimate: " << std::fixed << std::setprecision(1) << hll.estimate() 
              << " (Error: " << std::abs(hll.estimate() - true_cardinality) / true_cardinality * 100.0 << "%)\n\n";

    // ----------------------------------------------------------------
    // 4. Johnson-Lindenstrauss Random Projection
    // ----------------------------------------------------------------
    std::cout << "--- 4. Johnson-Lindenstrauss Random Projection ---\n";
    int original_dim = 1000;
    int target_dim = 150;
    std::cout << "  Projecting from R^" << original_dim << " to R^" << target_dim << "\n";

    // Generate two high-dimensional points
    std::vector<double> p1(original_dim, 0.0);
    std::vector<double> p2(original_dim, 0.0);
    std::uniform_real_distribution<double> val_dist(-10.0, 10.0);
    for (int i = 0; i < original_dim; ++i) {
        p1[i] = val_dist(rng);
        p2[i] = val_dist(rng);
    }

    ral::RandomProjection rp(original_dim, target_dim, rng);
    auto proj1 = rp.project(p1);
    auto proj2 = rp.project(p2);

    double dist_original = ral::RandomProjection::squared_distance(p1, p2);
    double dist_projected = ral::RandomProjection::squared_distance(proj1, proj2);

    std::cout << "  Original Squared Distance:  " << std::fixed << std::setprecision(2) << dist_original << "\n";
    std::cout << "  Projected Squared Distance: " << dist_projected 
              << " (Distortion: " << std::abs(dist_projected - dist_original) / dist_original * 100.0 << "%)\n\n";

    // ----------------------------------------------------------------
    // 5. Randomized Numerical Linear Algebra (Range Finder)
    // ----------------------------------------------------------------
    std::cout << "--- 5. Randomized Range Finder (RandNLA) ---\n";
    // Build a low-rank 100x100 matrix of rank 2
    // A = u1 * v1^T + u2 * v2^T
    int m = 100, n = 100;
    std::vector<double> u1(m), v1(n), u2(m), v2(n);
    std::normal_distribution<double> g(0.0, 1.0);
    for (int i = 0; i < m; ++i) { u1[i] = g(rng); u2[i] = g(rng); }
    for (int i = 0; i < n; ++i) { v1[i] = g(rng); v2[i] = g(rng); }

    ral::RandomizedLA::Matrix A(m, std::vector<double>(n, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            A[i][j] = u1[i] * v1[j] + u2[i] * v2[j];
        }
    }

    // Run randomized range finder to find a rank-2 approximation (using oversampling of 3, total rank = 5)
    auto Q = ral::RandomizedLA::randomized_range_finder(A, 2, 3, rng);
    
    // Project A onto Q: A_approx = Q * Q^T * A
    auto QT = ral::RandomizedLA::transpose(Q);
    auto QT_A = ral::RandomizedLA::multiply(QT, A);
    auto A_approx = ral::RandomizedLA::multiply(Q, QT_A);

    // Compute projection error norm: ||A - A_approx||_Frobenius
    double err_norm = 0.0;
    double orig_norm = 0.0;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            double diff = A[i][j] - A_approx[i][j];
            err_norm += diff * diff;
            orig_norm += A[i][j] * A[i][j];
        }
    }
    err_norm = std::sqrt(err_norm);
    orig_norm = std::sqrt(orig_norm);

    std::cout << "  Original Matrix Rank: 2 (Size: " << m << "x" << n << ")\n";
    std::cout << "  Orthonormal Basis Q Size: " << Q.size() << "x" << Q[0].size() << "\n";
    std::cout << "  Relative Reconstruction Error: " << std::fixed << std::setprecision(6) 
              << (err_norm / orig_norm) << " (Expected close to 0)\n";
    std::cout << "========================================================\n";

    return 0;
}
