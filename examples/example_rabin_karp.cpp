// =============================================================================
// Rabin-Karp Pattern Matching
//
// A randomized string-matching algorithm that uses rolling polynomial
// fingerprints. Compute hash of the pattern and of each text window;
// if hashes match, verify character-by-character (to avoid false positives).
//
// The rolling hash allows shifting the window in O(1):
//   hash(s[i..i+m-1]) = (hash(s[i-1..i+m-2]) - s[i-1]*h) * BASE + s[i+m-1]
// where h = BASE^(m-1) mod MOD.
//
// Expected time: O(n + m) for random text. Worst case: O(n*m).
// =============================================================================

#include "ral/rabin_karp.h"
#include <iostream>
#include <string>
#include <vector>
#include <random>

namespace ral {

inline void demonstrate_rabin_karp_detail() {
    std::cout << "=== Rabin-Karp Pattern Matching ===\n\n";

    // Test 1: overlapping pattern
    std::string text = "AABAACAADAABAABA";
    std::string pattern = "AABA";
    std::cout << "Text:    \"" << text << "\"\n";
    std::cout << "Pattern: \"" << pattern << "\"\n";
    auto matches = rabin_karp_search(text, pattern);
    std::cout << "Matches at: ";
    for (size_t i = 0; i < matches.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << matches[i];
    }
    std::cout << "\n\n";

    // Test 2: DNA sequence search
    std::string dna = "ATCGATCGATCGTAGCTAGCTAGCTAG";
    std::string motif = "TAGC";
    std::cout << "DNA:     \"" << dna << "\"\n";
    std::cout << "Motif:   \"" << motif << "\"\n";
    matches = rabin_karp_search(dna, motif);
    std::cout << "Matches at: ";
    for (size_t i = 0; i < matches.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << matches[i];
    }
    std::cout << "\n\n";

    // Test 3: no match
    std::string hay = "abcdefghij";
    std::string needle = "xyz";
    std::cout << "Text:    \"" << hay << "\"\n";
    std::cout << "Pattern: \"" << needle << "\"\n";
    matches = rabin_karp_search(hay, needle);
    std::cout << "Matches: " << (matches.empty() ? "(none)" : "unexpected") << "\n\n";

    // Test 4: pattern longer than text
    std::cout << "Pattern longer than text: ";
    matches = rabin_karp_search("abc", "abcdef");
    std::cout << (matches.empty() ? "(none, correct)" : "ERROR") << "\n\n";

    // Test 5: rolling hash demonstration
    std::cout << "--- Rolling Hash Mechanism ---\n";
    const long long BASE = 256;
    const long long MOD = 1000000007LL;
    std::string demo_text = "abcdef";
    int m = 3;
    std::cout << "Text: \"" << demo_text << "\",  window size: " << m << "\n";
    for (int i = 0; i <= (int)demo_text.size() - m; ++i) {
        std::string window = demo_text.substr(i, m);
        long long h = string_fingerprint(window, BASE, MOD);
        std::cout << "  window[" << i << "] = \"" << window << "\"  hash = " << h << "\n";
    }
}

} // namespace ral

int main() {
    using namespace ral;
    demonstrate_rabin_karp_detail();
    return 0;
}
