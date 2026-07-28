// example_simhash.cpp
// SimHash for Cosine Similarity LSH
//
// SimHash projects points onto k random hyperplanes, producing a k-bit
// signature. Two points with similar orientation (high cosine similarity)
// will have signatures with small Hamming distance. The probability that
// two signatures agree on a bit equals (1 - theta/pi) where theta is the
// angle between the vectors.
//
// Compile: g++ -std=c++23 -I../include -o example_simhash example_simhash.cpp

#include <ral/lsh.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>

using namespace ral;

double cosine_similarity(const std::vector<double>& a, const std::vector<double>& b) {
    double dot = 0.0, na = 0.0, nb = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        dot += a[i] * b[i];
        na += a[i] * a[i];
        nb += b[i] * b[i];
    }
    return dot / (std::sqrt(na) * std::sqrt(nb) + 1e-12);
}

int main() {
    std::mt19937 rng(42);

    std::cout << "=== SimHash (Cosine Similarity LSH) ===\n\n";

    int dim = 10;
    int k = 64;  // number of hash bits

    SimHash sh(dim, k, rng);

    // Create test vectors
    std::vector<double> v1 = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<double> v2 = {1, 0.1, 0, 0, 0, 0, 0, 0, 0, 0};  // similar to v1
    std::vector<double> v3 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1};    // orthogonal to v1
    std::vector<double> v4 = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0};    // 45 deg from v1

    auto sig1 = sh.hash(v1);
    auto sig2 = sh.hash(v2);
    auto sig3 = sh.hash(v3);
    auto sig4 = sh.hash(v4);

    std::cout << "Testing with k=" << k << " random hyperplanes, dim=" << dim << "\n\n";

    auto print_stats = [&](const std::string& label,
                           const std::vector<double>& a,
                           const std::vector<double>& b,
                           const std::vector<int>& sa,
                           const std::vector<int>& sb) {
        double cos_sim = cosine_similarity(a, b);
        int hamming = SimHash::hamming_distance(sa, sb);
        double agreement = 1.0 - (double)hamming / k;
        std::cout << "  " << label << ":\n";
        std::cout << "    Cosine similarity: " << cos_sim << "\n";
        std::cout << "    Hamming distance:  " << hamming << "/" << k << "\n";
        std::cout << "    Signature agreement: " << agreement << "\n";
        std::cout << "    Expected agreement ~ (1 - theta/pi): "
                  << (1.0 - std::acos(cos_sim) / M_PI) << "\n\n";
    };

    print_stats("v1 vs v2 (nearly identical)", v1, v2, sig1, sig2);
    print_stats("v1 vs v3 (orthogonal)", v1, v3, sig1, sig3);
    print_stats("v1 vs v4 (45 degrees apart)", v1, v4, sig1, sig4);

    return 0;
}
