#pragma once

#include <vector>
#include <random>
#include <cmath>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <functional>
#include "random_utils.h"
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Locality-Sensitive Hashing (LSH)
// A family H of hash functions is (r1, r2, p1, p2)-sensitive if:
//   - For ||x-y|| <= r1:  Pr[h(x) = h(y)] >= p1
//   - For ||x-y|| >= r2:  Pr[h(x) = h(y)] <= p2
//
// We implement:
//   1. SimHash (cosine similarity LSH)
//   2. Hyperplane LSH (angular distance)
//   3. E2LSH (p-stable distribution LSH for L2 distance)
// ---------------------------------------------------------------------------

struct LSHResult {
    int index;
    double distance;
};

// --- SimHash (Cosine Similarity LSH) ---

struct SimHash {
    std::vector<std::vector<double>> hyperplanes;  // k hyperplanes, each d-dimensional
    int k;

    SimHash(int dim, int num_hashes, std::mt19937& rng)
        : k(num_hashes) {
        std::normal_distribution<double> norm(0.0, 1.0);
        hyperplanes.resize(k, std::vector<double>(dim));
        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < dim; ++j) {
                hyperplanes[i][j] = norm(rng);
            }
        }
    }

    // Hash: sign of dot product with each hyperplane -> k-bit signature
    std::vector<int> hash(const std::vector<double>& point) const {
        std::vector<int> sig(k);
        for (int i = 0; i < k; ++i) {
            double dot = 0.0;
            for (size_t j = 0; j < point.size(); ++j) {
                dot += hyperplanes[i][j] * point[j];
            }
            sig[i] = (dot >= 0.0) ? 1 : 0;
        }
        return sig;
    }

    // Hamming distance between two signatures
    static int hamming_distance(const std::vector<int>& a, const std::vector<int>& b) {
        int d = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) d++;
        }
        return d;
    }
};

// --- Hyperplane LSH (Angular Distance) ---

struct HyperplaneLSH {
    std::vector<std::vector<double>> planes;
    int k;

    HyperplaneLSH(int dim, int num_hashes, std::mt19937& rng)
        : k(num_hashes) {
        std::normal_distribution<double> norm(0.0, 1.0);
        planes.resize(k, std::vector<double>(dim));
        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < dim; ++j) {
                planes[i][j] = norm(rng);
            }
        }
    }

    std::vector<int> hash(const std::vector<double>& point) const {
        std::vector<int> sig(k);
        for (int i = 0; i < k; ++i) {
            double dot = 0.0;
            for (size_t j = 0; j < point.size(); ++j) {
                dot += planes[i][j] * point[j];
            }
            sig[i] = (dot >= 0.0) ? 1 : 0;
        }
        return sig;
    }
};

// --- E2LSH (p-stable distribution LSH for L2) ---

struct E2LSH {
    std::vector<std::vector<double>> a;  // random vectors
    std::vector<double> b;               // random offsets
    double w;                            // bucket width
    int k;                               // number of hash functions

    E2LSH(int dim, int num_hashes, double bucket_width, std::mt19937& rng)
        : w(bucket_width), k(num_hashes) {
        std::normal_distribution<double> norm(0.0, 1.0);
        std::uniform_real_distribution<double> unif(0.0, bucket_width);

        a.resize(k, std::vector<double>(dim));
        b.resize(k);
        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < dim; ++j) {
                a[i][j] = norm(rng);
            }
            b[i] = unif(rng);
        }
    }

    // Hash: floor((a . x + b) / w) for each hash function
    std::vector<long long> hash(const std::vector<double>& point) const {
        std::vector<long long> sig(k);
        for (int i = 0; i < k; ++i) {
            double dot = b[i];
            for (size_t j = 0; j < point.size(); ++j) {
                dot += a[i][j] * point[j];
            }
            sig[i] = static_cast<long long>(std::floor(dot / w));
        }
        return sig;
    }
};

// --- LSH Index: stores points with their hash signatures ---

class LSHIndex {
public:
    // Signature vector -> list of (original_index, point) pairs
    std::unordered_map<std::string, std::vector<int>> buckets;
    std::vector<std::vector<double>> points;
    int dim;
    int k;

    LSHIndex(int dim, int num_hashes, std::mt19937& rng)
        : dim(dim), k(num_hashes), hash_proj(dim, num_hashes, rng) {}

    void insert(const std::vector<double>& point, int index) {
        points.push_back(point);
        auto sig = hash_proj.hash(point);
        buckets[key(sig)].push_back(index);
    }

    // Query: find approximate nearest neighbors
    std::vector<LSHResult> query(const std::vector<double>& query_point, int max_results = 10) const {
        auto sig = hash_proj.hash(query_point);
        auto it = buckets.find(key(sig));

        std::vector<LSHResult> results;
        if (it != buckets.end()) {
            for (int idx : it->second) {
                double d = euclidean_distance(query_point, points[idx]);
                results.push_back({idx, d});
            }
        }

        // Sort by distance
        std::sort(results.begin(), results.end(),
                  [](const LSHResult& a, const LSHResult& b) {
                      return a.distance < b.distance;
                  });

        if (static_cast<int>(results.size()) > max_results) {
            results.resize(max_results);
        }
        return results;
    }

private:
    SimHash hash_proj;

    static std::string key(const std::vector<int>& sig) {
        std::string s;
        for (int b : sig) s += std::to_string(b);
        return s;
    }

    static std::string key(const std::vector<long long>& sig) {
        std::string s;
        for (long long b : sig) s += std::to_string(b) + ",";
        return s;
    }

    static double euclidean_distance(const std::vector<double>& a, const std::vector<double>& b) {
        double d = 0.0;
        for (size_t i = 0; i < a.size(); ++i) {
            double diff = a[i] - b[i];
            d += diff * diff;
        }
        return std::sqrt(d);
    }
};

// --- Brute force nearest neighbor for comparison ---

inline std::vector<LSHResult> brute_force_nn(
    const std::vector<std::vector<double>>& points,
    const std::vector<double>& query, int max_results) {

    std::vector<LSHResult> results;
    for (size_t i = 0; i < points.size(); ++i) {
        double d = 0.0;
        for (size_t j = 0; j < query.size(); ++j) {
            double diff = query[j] - points[i][j];
            d += diff * diff;
        }
        d = std::sqrt(d);
        results.push_back({static_cast<int>(i), d});
    }
    std::sort(results.begin(), results.end(),
              [](const LSHResult& a, const LSHResult& b) {
                  return a.distance < b.distance;
              });
    if (static_cast<int>(results.size()) > max_results) {
        results.resize(max_results);
    }
    return results;
}

// Demonstration
inline void demonstrate_lsh() {
    std::mt19937 rng(42);

    println("=== Locality-Sensitive Hashing ===\n");

    // Generate random points in high dimension
    int dim = 20;
    int n = 1000;
    int num_hashes = 16;

    std::normal_distribution<double> norm(0.0, 1.0);
    std::vector<std::vector<double>> data(n, std::vector<double>(dim));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < dim; ++j) {
            data[i][j] = norm(rng);
        }
    }

    // Build LSH index
    LSHIndex index(dim, num_hashes, rng);
    for (int i = 0; i < n; ++i) {
        index.insert(data[i], i);
    }

    // Query
    std::vector<double> query(dim);
    for (int j = 0; j < dim; ++j) query[j] = norm(rng);

    println("--- LSH Approximate Nearest Neighbor (n={}, dim={}, k={}) ---", n, dim, num_hashes);

    auto lsh_results = index.query(query, 5);
    auto bf_results = brute_force_nn(data, query, 5);

    println("  LSH results:");
    for (auto& r : lsh_results) {
        println("    index={}, distance={:.4f}", r.index, r.distance);
    }
    println("  Brute-force results:");
    for (auto& r : bf_results) {
        println("    index={}, distance={:.4f}", r.index, r.distance);
    }

    // Measure recall
    int correct = 0;
    for (auto& lr : lsh_results) {
        for (auto& br : bf_results) {
            if (lr.index == br.index) {
                correct++;
                break;
            }
        }
    }
    println("  Recall@5: {:.1f}%", 100.0 * correct / 5);

    // E2LSH demo
    println("\n--- E2LSH (L2 Distance, bucket_width=2.0) ---");
    E2LSH e2lsh(dim, num_hashes, 2.0, rng);
    // Just demonstrate hashing
    auto e2sig = e2lsh.hash(query);
    print("  Query hash signature: [");
    for (size_t i = 0; i < std::min<size_t>(8, e2sig.size()); ++i) {
        if (i) print(", ");
        print("{}", e2sig[i]);
    }
    println(", ...]");

    // Performance test
    println("\n--- Scalability ---");
    for (int sz : {100, 1000, 10000}) {
        LSHIndex idx(dim, num_hashes, rng);
        std::vector<std::vector<double>> pts(sz, std::vector<double>(dim));
        for (int i = 0; i < sz; ++i) {
            for (int j = 0; j < dim; ++j) pts[i][j] = norm(rng);
            idx.insert(pts[i], i);
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        for (int q = 0; q < 100; ++q) {
            idx.query(pts[q % sz], 5);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        println("  n={}: 100 queries in {:.3f} ms ({:.3f} ms/query)",
                      sz, ms, ms / 100);
    }
}

} // namespace ral
