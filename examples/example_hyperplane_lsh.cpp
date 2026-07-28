// example_hyperplane_lsh.cpp
// Hyperplane LSH for Angular Distance
//
// Similar to SimHash but used in the LSHIndex for nearest neighbor search.
// Random hyperplanes partition the space into half-spaces. Points on the
// same side of all k hyperplanes get the same hash, forming a bucket.
// This gives a (r, cr, p1, p2)-sensitive family for cosine distance.
//
// Compile: g++ -std=c++23 -I../include -o example_hyperplane_lsh example_hyperplane_lsh.cpp

#include <ral/lsh.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>

using namespace ral;

int main() {
    std::mt19937 rng(42);

    std::cout << "=== Hyperplane LSH for Cosine Similarity ===\n\n";

    int dim = 8;
    int k = 16;

    HyperplaneLSH hlsh(dim, k, rng);

    // Create vectors
    std::vector<double> a = {1, 0, 0, 0, 0, 0, 0, 0};
    std::vector<double> b = {0.9, 0.1, 0.2, 0, 0, 0, 0, 0};  // close to a
    std::vector<double> c = {0, 0, 0, 0, 0, 0, 0, 1};         // far from a

    auto sig_a = hlsh.hash(a);
    auto sig_b = hlsh.hash(b);
    auto sig_c = hlsh.hash(c);

    auto print_sig = [](const std::string& name, const std::vector<int>& sig) {
        std::cout << "  " << name << ": ";
        for (int b : sig) std::cout << b;
        std::cout << "\n";
    };

    std::cout << "Hash signatures (k=" << k << " hyperplanes):\n";
    print_sig("a = (1,0,...,0)", sig_a);
    print_sig("b = (0.9,0.1,...)", sig_b);
    print_sig("c = (0,...,0,1)", sig_c);

    // Hamming distances
    std::cout << "\nHamming distances:\n";
    std::cout << "  a vs b: distance = "
              << SimHash::hamming_distance(sig_a, sig_b) << "/" << k << "\n";
    std::cout << "  a vs c: distance = "
              << SimHash::hamming_distance(sig_a, sig_c) << "/" << k << "\n";
    std::cout << "  b vs c: distance = "
              << SimHash::hamming_distance(sig_b, sig_c) << "/" << k << "\n";

    // Build an index and do ANN search
    std::cout << "\n--- ANN Search with Hyperplane LSH Index ---\n";

    int n = 500;
    std::normal_distribution<double> norm(0.0, 1.0);

    // Generate clustered data: two clusters
    std::vector<std::vector<double>> data(n, std::vector<double>(dim));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < dim; ++j) {
            data[i][j] = (i < n / 2) ? norm(rng) + 3.0 : norm(rng) - 3.0;
        }
    }

    LSHIndex index(dim, k, rng);
    for (int i = 0; i < n; ++i) {
        index.insert(data[i], i);
    }

    // Query from cluster 0
    std::vector<double> query(dim);
    for (int j = 0; j < dim; ++j) query[j] = norm(rng) + 3.0;

    auto results = index.query(query, 5);
    auto bf = brute_force_nn(data, query, 5);

    std::cout << "Query from cluster 0, top 5 nearest neighbors:\n";
    std::cout << "  LSH:  ";
    for (auto& r : results) std::cout << "idx=" << r.index << "(d=" << r.distance << ") ";
    std::cout << "\n";
    std::cout << "  BF:   ";
    for (auto& r : bf) std::cout << "idx=" << r.index << "(d=" << r.distance << ") ";
    std::cout << "\n";

    int correct = 0;
    for (auto& lr : results) {
        for (auto& br : bf) {
            if (lr.index == br.index) { correct++; break; }
        }
    }
    std::cout << "  Recall@5: " << 100.0 * correct / 5 << "%\n";

    return 0;
}
