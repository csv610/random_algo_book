// example_e2lsh.cpp
// E2LSH (p-stable Distribution LSH for L2/Euclidean Distance)
//
// Uses hash functions h(x) = floor((a . x + b) / w) where a is drawn
// from a standard normal (a p-stable distribution for L2) and b is
// uniform in [0, w]. Points close in Euclidean distance hash to the
// same bucket with probability decreasing with distance. The bucket
// width w controls the trade-off between precision and recall.
//
// Compile: g++ -std=c++23 -I../include -o example_e2lsh example_e2lsh.cpp

#include <ral/lsh.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>

using namespace ral;

double euclidean_dist(const std::vector<double>& a, const std::vector<double>& b) {
    double d = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return std::sqrt(d);
}

int main() {
    std::mt19937 rng(42);

    std::cout << "=== E2LSH (Euclidean Distance LSH) ===\n\n";

    int dim = 5;
    int k = 8;
    double w = 2.0;

    E2LSH e2lsh(dim, k, w, rng);

    // Test with known points
    std::vector<double> p1 = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> p2 = {1.1, 2.1, 3.1, 4.1, 5.1};  // close to p1
    std::vector<double> p3 = {10.0, 20.0, 30.0, 40.0, 50.0};  // far from p1

    auto sig1 = e2lsh.hash(p1);
    auto sig2 = e2lsh.hash(p2);
    auto sig3 = e2lsh.hash(p3);

    std::cout << "Bucket width w = " << w << ", k = " << k << " hash functions\n\n";

    auto print_sig = [](const std::string& label, const std::vector<long long>& sig) {
        std::cout << "  " << label << ": [";
        for (size_t i = 0; i < sig.size(); ++i) {
            if (i) std::cout << ", ";
            std::cout << sig[i];
        }
        std::cout << "]\n";
    };

    print_sig("p1 = (1,2,3,4,5)", sig1);
    print_sig("p2 = (1.1,2.1,3.1,4.1,5.1)", sig2);
    print_sig("p3 = (10,20,30,40,50)", sig3);

    std::cout << "\nEuclidean distances:\n";
    std::cout << "  p1 vs p2: " << euclidean_dist(p1, p2) << "\n";
    std::cout << "  p1 vs p3: " << euclidean_dist(p1, p3) << "\n";

    // Count matching hash values
    int match_12 = 0, match_13 = 0;
    for (int i = 0; i < k; ++i) {
        if (sig1[i] == sig2[i]) match_12++;
        if (sig1[i] == sig3[i]) match_13++;
    }
    std::cout << "\nHash agreement (out of " << k << " functions):\n";
    std::cout << "  p1 vs p2: " << match_12 << "/" << k << "\n";
    std::cout << "  p1 vs p3: " << match_13 << "/" << k << "\n";

    // Demonstrate effect of bucket width
    std::cout << "\n--- Effect of Bucket Width ---\n";
    for (double bw : {0.5, 1.0, 2.0, 5.0, 10.0}) {
        E2LSH lsh_test(dim, k, bw, rng);
        auto s1 = lsh_test.hash(p1);
        auto s2 = lsh_test.hash(p2);
        int match = 0;
        for (int i = 0; i < k; ++i) {
            if (s1[i] == s2[i]) match++;
        }
        std::cout << "  w=" << bw << ": " << match << "/" << k << " matching hashes for close points\n";
    }

    return 0;
}
