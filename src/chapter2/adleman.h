#pragma once
// Adleman's Theorem: BPP with non-uniform advice
// Demonstrates the concept of non-uniform computation

#include <vector>
#include <random>
#include <iostream>
#include <bitset>
#include <chrono>

namespace chapter2 {

// Simulate a BPP algorithm for a unary language
// Language L = {1^n : n is prime} (simplified example)
class BPPLanguage {
public:
    BPPLanguage() : rng_(42) {}

    // Deterministic check if n is prime
    bool is_prime(int n) const {
        if (n < 2) return false;
        if (n == 2 || n == 3) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;
        for (int i = 5; i * i <= n; i += 6)
            if (n % i == 0 || n % (i + 2) == 0)
                return false;
        return true;
    }

    // BPP algorithm: use random sampling for primality
    // (Solovay-Strassen style, simplified)
    bool bpp_check(int n, int t = 10) {
        if (n < 2) return false;
        if (n == 2 || n == 3) return true;
        if (n % 2 == 0) return false;

        std::uniform_int_distribution<long long> dist(2, n - 1);
        for (int i = 0; i < t; ++i) {
            long long a = dist(rng_);
            if (gcd(a, n) != 1) return false;
            if (powmod(a, (n - 1) / 2, n) != legendre(a, n))
                return false;
        }
        return true;
    }

    // Non-uniform advice: for each input length n,
    // the advice string encodes a "good" set of witnesses
    struct Advice {
        std::vector<int> witness_bits;
    };

    // Generate advice for input length n
    // In Adleman's theorem, this advice helps the algorithm
    Advice generate_advice(int n) const {
        Advice adv;
        // For small n, encode the answer directly
        adv.witness_bits.resize(n + 1);
        for (int i = 0; i <= n; ++i)
            adv.witness_bits[i] = is_prime(i) ? 1 : 0;
        return adv;
    }

    // Non-uniform algorithm: uses advice
    bool nonuniform_check(int n, const Advice& adv) {
        if (n < 0 || n >= static_cast<int>(adv.witness_bits.size()))
            return bpp_check(n);
        return adv.witness_bits[n] == 1;
    }

private:
    mutable std::mt19937 rng_;

    long long gcd(long long a, long long b) const {
        while (b) { a %= b; std::swap(a, b); }
        return a;
    }

    long long powmod(long long base, long long exp, long long mod) const {
        long long result = 1;
        base %= mod;
        while (exp > 0) {
            if (exp & 1) result = (result * base) % mod;
            base = (base * base) % mod;
            exp >>= 1;
        }
        return result;
    }

    int legendre(long long a, long long p) const {
        long long lp = powmod(a, (p - 1) / 2, p);
        return (lp == p - 1) ? -1 : static_cast<int>(lp);
    }
};

void demonstrate_adleman() {
    std::cout << "Adleman's Theorem: BPP with Non-Uniform Advice\n\n";
    std::cout << "Theorem: BPP <= P/poly\n";
    std::cout << "Any BPP language has polynomial-size advice for each input length.\n\n";

    BPPLanguage lang;

    std::cout << "Primality testing (simplified):\n";
    std::cout << "  BPP algorithm uses O(log n) random bits\n";
    std::cout << "  Error probability: <= 2^{-t} with t trials\n\n";

    // Compare BPP and non-uniform approaches
    std::cout << "Comparison for n = 2 to 30:\n";
    std::cout << "  n   prime?  BPP(10)  advice\n";
    for (int n = 2; n <= 30; ++n) {
        bool actual = lang.is_prime(n);
        bool bpp = lang.bpp_check(n, 10);
        auto adv = lang.generate_advice(n);
        bool nonunif = lang.nonuniform_check(n, adv);
        std::cout << "  " << n << "   "
                  << (actual ? "  yes" : "   no") << "    "
                  << (bpp ? " yes" : "  no") << "    "
                  << (nonunif ? " yes" : "  no") << "\n";
    }
    std::cout << "\n";

    std::cout << "Key insight: Non-uniform advice depends only on input length,\n";
    std::cout << "not the input itself. This makes P/poly strictly larger than BPP\n";
    std::cout << "(unless polynomial hierarchies collapse).\n\n";
}

}  // namespace chapter2
