// =============================================================================
// String Fingerprinting for Equality Testing
//
// A Karp-Rabin fingerprint maps a string s to a single integer:
//   f(s) = s[0]*base^(n-1) + s[1]*base^(n-2) + ... + s[n-1]  (mod p)
//
// If f(s1) != f(s2), the strings are definitely different.
// If f(s1) == f(s2), they are likely the same (collision prob <= n/p
// for a degree-(n-1) polynomial over Z_p).
//
// This is useful for deduplication, caching, and set membership testing.
// =============================================================================

#include "ral/rabin_karp.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <random>

namespace ral {

inline void demonstrate_fingerprint() {
    std::cout << "=== String Fingerprinting for Equality Testing ===\n\n";

    const long long BASE = 256;
    const long long MOD = 1000000007LL;

    // Basic fingerprint properties
    std::cout << "--- Fingerprint Values ---\n";
    std::string words[] = {"hello", "world", "hello", "abc", "abd"};
    for (const auto& w : words) {
        std::cout << "  f(\"" << w << "\") = " << string_fingerprint(w, BASE, MOD) << "\n";
    }

    // Demonstrate: same string => same fingerprint
    std::cout << "\n--- Determinism ---\n";
    std::string s = "algorithm";
    long long h1 = string_fingerprint(s, BASE, MOD);
    long long h2 = string_fingerprint(s, BASE, MOD);
    std::cout << "  f(\"" << s << "\") twice: " << h1 << " == " << h2
              << " => " << (h1 == h2 ? "same (correct)" : "ERROR") << "\n";

    // Deduplication example
    std::cout << "\n--- Deduplication via Fingerprints ---\n";
    std::vector<std::string> documents = {
        "the quick brown fox",
        "lazy dogs and cats",
        "the quick brown fox",
        "a different document",
        "lazy dogs and cats"
    };

    std::unordered_set<long long> seen;
    int unique = 0;
    for (const auto& doc : documents) {
        long long fp = string_fingerprint(doc, BASE, MOD);
        if (seen.insert(fp).second) {
            unique++;
            std::cout << "  NEW:   \"" << doc << "\" (fp=" << fp << ")\n";
        } else {
            std::cout << "  DUP:   \"" << doc << "\" (fp=" << fp << ")\n";
        }
    }
    std::cout << "  Total: " << documents.size() << " docs, " << unique << " unique\n";

    // Collision probability analysis
    std::cout << "\n--- Collision Probability ---\n";
    std::cout << "  For strings of length n over Z_p:\n";
    std::cout << "  Pr[f(s1) = f(s2] | s1 != s2] <= n / p\n";
    for (int n : {10, 50, 100, 1000}) {
        double bound = static_cast<double>(n) / MOD;
        std::cout << "    n=" << n << ":  <= " << bound << "\n";
    }

    // Empirical collision test: generate random strings, count collisions
    std::cout << "\n--- Empirical Collision Test ---\n";
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> len_dist(5, 15);
    std::uniform_int_distribution<int> char_dist('a', 'z');

    int num_strings = 10000;
    std::unordered_set<long long> fingerprints;
    int collisions = 0;
    for (int i = 0; i < num_strings; ++i) {
        int len = len_dist(rng);
        std::string rand_str(len, ' ');
        for (char& c : rand_str) c = static_cast<char>(char_dist(rng));
        long long fp = string_fingerprint(rand_str, BASE, MOD);
        if (!fingerprints.insert(fp).second) {
            collisions++;
        }
    }
    std::cout << "  " << num_strings << " random strings, collisions: " << collisions << "\n";
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_fingerprint();
    return 0;
}
