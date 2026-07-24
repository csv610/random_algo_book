#pragma once

#include <vector>
#include <random>
#include <cmath>
#include <stdexcept>

namespace ral {

// ============================================================
// Johnson-Lindenstrauss Random Projection
// Projects points from R^d down to R^k (where k << d)
// preserving pairwise Euclidean distances with high probability.
// ============================================================
class RandomProjection {
private:
    int original_dim_;
    int target_dim_;
    std::vector<std::vector<double>> projection_matrix_;

public:
    // Generate a k x d random Gaussian projection matrix
    RandomProjection(int original_dim, int target_dim, std::mt19937& rng)
        : original_dim_(original_dim), target_dim_(target_dim) {
        if (original_dim <= 0 || target_dim <= 0) {
            throw std::invalid_argument("Dimensions must be positive.");
        }

        // Standard scaling factor for distance preservation is 1/sqrt(k)
        double scale = 1.0 / std::sqrt(static_cast<double>(target_dim));
        std::normal_distribution<double> gaussian(0.0, 1.0);

        projection_matrix_.assign(target_dim_, std::vector<double>(original_dim_));
        for (int i = 0; i < target_dim_; ++i) {
            for (int j = 0; j < original_dim_; ++j) {
                projection_matrix_[i][j] = gaussian(rng) * scale;
            }
        }
    }

    // Project a single high-dimensional point to the target low-dimensional space
    std::vector<double> project(const std::vector<double>& point) const {
        if (static_cast<int>(point.size()) != original_dim_) {
            throw std::invalid_argument("Point dimension does not match projection original dimension.");
        }

        std::vector<double> projected(target_dim_, 0.0);
        for (int i = 0; i < target_dim_; ++i) {
            double sum = 0.0;
            for (int j = 0; j < original_dim_; ++j) {
                sum += projection_matrix_[i][j] * point[j];
            }
            projected[i] = sum;
        }
        return projected;
    }

    // Project a dataset of multiple high-dimensional points
    std::vector<std::vector<double>> project_dataset(const std::vector<std::vector<double>>& dataset) const {
        std::vector<std::vector<double>> projected_dataset;
        projected_dataset.reserve(dataset.size());
        for (const auto& point : dataset) {
            projected_dataset.push_back(project(point));
        }
        return projected_dataset;
    }

    // Helper: Compute squared Euclidean distance between two vectors
    static double squared_distance(const std::vector<double>& v1, const std::vector<double>& v2) {
        if (v1.size() != v2.size()) {
            throw std::invalid_argument("Vector dimensions must match to compute distance.");
        }
        double sum = 0.0;
        for (size_t i = 0; i < v1.size(); ++i) {
            double diff = v1[i] - v2[i];
            sum += diff * diff;
        }
        return sum;
    }
};

} // namespace ral
