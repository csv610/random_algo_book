// example_lsh_index.cpp
// LSH Index for Approximate Nearest Neighbor Search
//
// Builds an LSH index using SimHash projections. Points are inserted
// into buckets based on their k-bit hash signature. Queries hash the
// query point and return candidates from the same bucket, sorted by
// Euclidean distance. Compare with brute-force for recall measurement.
//
// Compile: g++ -std=c++23 -I../include -o example_lsh_index example_lsh_index.cpp

#include <ral/lsh.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== LSH Index for Nearest Neighbor Search ===\n\n";

    int dim = 10;
    int n = 500;
    int k = 16;

    // Generate data with some structure: two clusters
    std::normal_distribution<double> norm(0.0, 1.0);
    std::vector<std::vector<double>> data(n, std::vector<double>(dim));
    for (int i = 0; i < n; ++i) {
        double offset = (i < n / 2) ? 5.0 : -5.0;
        for (int j = 0; j < dim; ++j) {
            data[i][j] = norm(rng) + offset;
        }
    }

    // Build index
    LSHIndex index(dim, k, rng);
    for (int i = 0; i < n; ++i) {
        index.insert(data[i], i);
    }

    std::cout << "Indexed " << n << " points in " << dim << " dimensions\n";
    std::cout << "Using " << k << " SimHash projections\n\n";

    // Run several queries and compare with brute force
    int queries = 20;
    double total_recall = 0.0;
    double lsh_time = 0.0, bf_time = 0.0;
    int top_k = 5;

    for (int q = 0; q < queries; ++q) {
        std::vector<double> query(dim);
        for (int j = 0; j < dim; ++j) query[j] = norm(rng);

        auto t1 = std::chrono::high_resolution_clock::now();
        auto lsh_results = index.query(query, top_k);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto bf_results = brute_force_nn(data, query, top_k);
        auto t3 = std::chrono::high_resolution_clock::now();

        lsh_time += std::chrono::duration<double, std::milli>(t2 - t1).count();
        bf_time += std::chrono::duration<double, std::milli>(t3 - t2).count();

        // Compute recall: how many of the top-k are in both result sets
        int correct = 0;
        for (auto& lr : lsh_results) {
            for (auto& br : bf_results) {
                if (lr.index == br.index) { correct++; break; }
            }
        }
        total_recall += (double)correct / top_k;
    }

    std::cout << "Results over " << queries << " queries (top-" << top_k << "):\n";
    std::cout << "  Average recall: " << 100.0 * total_recall / queries << "%\n";
    std::cout << "  LSH total time:  " << lsh_time << " ms\n";
    std::cout << "  Brute-force time: " << bf_time << " ms\n";
    std::cout << "  Speedup: " << bf_time / (lsh_time + 1e-9) << "x\n\n";

    // Show one detailed example
    std::vector<double> query(dim);
    for (int j = 0; j < dim; ++j) query[j] = norm(rng);

    auto lsh_results = index.query(query, 5);
    auto bf_results = brute_force_nn(data, query, 5);

    std::cout << "Detailed example query:\n";
    std::cout << "  LSH nearest neighbors:\n";
    for (auto& r : lsh_results) {
        std::cout << "    idx=" << r.index << "  distance=" << r.distance << "\n";
    }
    std::cout << "  Brute-force nearest neighbors:\n";
    for (auto& r : bf_results) {
        std::cout << "    idx=" << r.index << "  distance=" << r.distance << "\n";
    }

    // Scalability test
    std::cout << "\n--- Scalability ---\n";
    for (int sz : {100, 1000, 5000}) {
        LSHIndex idx(dim, k, rng);
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
        std::cout << "  n=" << sz << ": 100 queries in " << ms
                  << " ms (" << ms / 100 << " ms/query)\n";
    }

    return 0;
}
