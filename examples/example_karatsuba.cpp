// =============================================================================
// Karatsuba Fast Polynomial / Integer Multiplication
//
// Divide-and-conquer multiplication that splits each n-digit number into
// two halves and computes the product using 3 recursive multiplications
// instead of 4, achieving O(n^1.585) time.
//
// Given x = x1*B^m + x0, y = y1*B^m + y0:
//   z2 = x1 * y1
//   z0 = x0 * y0
//   z1 = (x1+x0)*(y1+y0) - z2 - z0
//   result = z2*B^(2m) + z1*B^m + z0
//
// This example applies Karatsuba to polynomial multiplication over Z_p,
// using the ral::Polynomial class.
// =============================================================================

#include "ral/polynomial.h"
#include <iostream>
#include <vector>

namespace ral {

// Karatsuba multiplication for polynomial coefficient vectors (mod p)
inline std::vector<long long> karatsuba(const std::vector<long long>& a,
                                        const std::vector<long long>& b,
                                        long long p) {
    int n = static_cast<int>(std::max(a.size(), b.size()));
    if (n <= 4) {
        // Base case: naive multiplication
        std::vector<long long> result(a.size() + b.size() - 1, 0);
        for (size_t i = 0; i < a.size(); ++i) {
            for (size_t j = 0; j < b.size(); ++j) {
                result[i + j] = (result[i + j] + a[i] * b[j]) % p;
            }
        }
        return result;
    }

    // Pad to even length
    n = ((n + 1) / 2) * 2;

    std::vector<long long> a_lo(n / 2, 0), a_hi(n / 2, 0);
    std::vector<long long> b_lo(n / 2, 0), b_hi(n / 2, 0);

    for (int i = 0; i < n / 2; ++i) {
        a_lo[i] = (i < (int)a.size()) ? a[i] : 0;
        a_hi[i] = (i + n / 2 < (int)a.size()) ? a[i + n / 2] : 0;
        b_lo[i] = (i < (int)b.size()) ? b[i] : 0;
        b_hi[i] = (i + n / 2 < (int)b.size()) ? b[i + n / 2] : 0;
    }

    // z2 = a_hi * b_hi
    auto z2 = karatsuba(a_hi, b_hi, p);
    // z0 = a_lo * b_lo
    auto z0 = karatsuba(a_lo, b_lo, p);

    // a_sum = a_lo + a_hi, b_sum = b_lo + b_hi
    std::vector<long long> a_sum(n / 2), b_sum(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        a_sum[i] = (a_lo[i] + a_hi[i]) % p;
        b_sum[i] = (b_lo[i] + b_hi[i]) % p;
    }

    // z1 = a_sum * b_sum - z2 - z0
    auto z1 = karatsuba(a_sum, b_sum, p);
    int z1_len = static_cast<int>(z1.size());
    int z2_len = static_cast<int>(z2.size());
    int z0_len = static_cast<int>(z0.size());

    // Adjust z1 length
    while ((int)z1.size() < std::max(z2_len, z0_len)) {
        z1.push_back(0);
    }

    for (int i = 0; i < (int)z2.size(); ++i) {
        z1[i] = (z1[i] - z2[i] + p) % p;
    }
    for (int i = 0; i < (int)z0.size(); ++i) {
        z1[i] = (z1[i] - z0[i] + p) % p;
    }

    // Combine: result = z0 + z1 * B^(n/2) + z2 * B^n
    int total_len = (int)z0.size();
    while (total_len < (int)z2.size() + n) total_len++;
    std::vector<long long> result(total_len, 0);

    for (int i = 0; i < (int)z0.size(); ++i) {
        result[i] = (result[i] + z0[i]) % p;
    }
    for (int i = 0; i < (int)z1.size(); ++i) {
        result[i + n / 2] = (result[i + n / 2] + z1[i]) % p;
    }
    for (int i = 0; i < (int)z2.size(); ++i) {
        result[i + n] = (result[i + n] + z2[i]) % p;
    }

    return result;
}

inline void demonstrate_karatsuba() {
    const long long P = 97;

    std::cout << "=== Karatsuba Polynomial Multiplication (mod " << P << ") ===\n\n";

    // Test 1: small polynomials
    std::vector<long long> a1 = {1, 2, 3};  // 1 + 2x + 3x^2
    std::vector<long long> b1 = {4, 5, 6};  // 4 + 5x + 6x^2

    auto k_prod = karatsuba(a1, b1, P);
    Polynomial pa(a1), pb(b1);
    auto n_prod = pa.multiply(pb, P);

    Polynomial pk(k_prod);
    std::cout << "A(x)     = " << pa.to_string() << "\n";
    std::cout << "B(x)     = " << pb.to_string() << "\n";
    std::cout << "Karatsuba= " << pk.to_string() << "\n";
    std::cout << "Naive    = " << n_prod.to_string() << "\n";
    std::cout << "Match?   " << (pk.to_string() == n_prod.to_string() ? "YES" : "NO") << "\n\n";

    // Test 2: larger polynomials (degree ~16)
    std::vector<long long> a2(17), b2(17);
    for (int i = 0; i < 17; ++i) {
        a2[i] = (i * 3 + 1) % P;
        b2[i] = (i * 7 + 5) % P;
    }
    Polynomial pa2(a2), pb2(b2);
    auto k2 = karatsuba(a2, b2, P);
    auto n2 = pa2.multiply(pb2, P);
    Polynomial pk2(k2);

    std::cout << "Degree-16 poly product match: "
              << (pk2.to_string() == n2.to_string() ? "YES" : "NO") << "\n";

    // Test 3: verify via evaluation at random point
    std::mt19937 rng(42);
    std::uniform_int_distribution<long long> dist(0, P - 1);
    long long r = dist(rng);
    std::cout << "Evaluate at x=" << r << ": Karatsuba=" << pk2.evaluate(r, P)
              << ", Naive=" << n2.evaluate(r, P) << "\n";
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_karatsuba();
    return 0;
}
