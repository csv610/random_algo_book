#pragma once
#include <string>
#include <vector>
#include <random>
#include <iostream>
#include <cassert>

namespace chapter8 {

// Compute Karp-Rabin fingerprint of string s
// f(s) = s[0]*base^(n-1) + s[1]*base^(n-2) + ... + s[n-1] (mod mod)
long long string_fingerprint(const std::string& s, long long base, long long mod) {
    long long hash = 0;
    for (char c : s) {
        hash = (hash * base + (unsigned char)c) % mod;
    }
    return hash;
}

// Compute base^(len-1) mod mod (needed for rolling hash)
long long power_mod(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) {
            result = result * base % mod;
        }
        base = base * base % mod;
        exp >>= 1;
    }
    return result;
}

// Rabin-Karp pattern matching
// Returns all starting indices where pattern occurs in text
std::vector<int> rabin_karp_search(const std::string& text, const std::string& pattern) {
    std::vector<int> matches;
    int n = (int)text.size();
    int m = (int)pattern.size();

    if (m > n || m == 0) return matches;

    const long long BASE = 256;
    const long long MOD = 1000000007LL;

    // Compute pattern fingerprint
    long long pat_hash = string_fingerprint(pattern, BASE, MOD);

    // Compute highest power: BASE^(m-1) mod MOD
    long long h = power_mod(BASE, m - 1, MOD);

    // Compute initial window fingerprint
    long long text_hash = 0;
    for (int i = 0; i < m; i++) {
        text_hash = (text_hash * BASE + (unsigned char)text[i]) % MOD;
    }

    // Slide the pattern over text
    for (int i = 0; i <= n - m; i++) {
        // Check if fingerprints match
        if (text_hash == pat_hash) {
            // Verify character by character (to avoid false positives)
            bool match = true;
            for (int j = 0; j < m; j++) {
                if (text[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                matches.push_back(i);
            }
        }

        // Compute hash for next window: remove leading char, add trailing char
        if (i < n - m) {
            text_hash = (text_hash - (unsigned char)text[i] * h % MOD + MOD) % MOD;
            text_hash = (text_hash * BASE + (unsigned char)text[i + m]) % MOD;
        }
    }

    return matches;
}

void demonstrate_rabin_karp() {
    std::cout << "=== Rabin-Karp Pattern Matching ===\n\n";

    // Test 1: Basic matching
    std::string text = "ABABDABACDABABCABAB";
    std::string pattern = "ABABCABAB";

    std::cout << "Text:    \"" << text << "\"\n";
    std::cout << "Pattern: \"" << pattern << "\"\n";

    std::vector<int> matches = rabin_karp_search(text, pattern);
    std::cout << "Matches at positions: ";
    if (matches.empty()) {
        std::cout << "(none)";
    } else {
        for (int i = 0; i < (int)matches.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << matches[i];
        }
    }
    std::cout << "\n\n";

    // Test 2: Multiple occurrences
    std::string text2 = "AAAAAA";
    std::string pattern2 = "AA";

    std::cout << "Text:    \"" << text2 << "\"\n";
    std::cout << "Pattern: \"" << pattern2 << "\"\n";

    matches = rabin_karp_search(text2, pattern2);
    std::cout << "Matches at positions: ";
    for (int i = 0; i < (int)matches.size(); i++) {
        if (i > 0) std::cout << ", ";
        std::cout << matches[i];
    }
    std::cout << "\n\n";

    // Test 3: No match
    std::string text3 = "ABCDEFGHIJ";
    std::string pattern3 = "XYZ";

    std::cout << "Text:    \"" << text3 << "\"\n";
    std::cout << "Pattern: \"" << pattern3 << "\"\n";

    matches = rabin_karp_search(text3, pattern3);
    std::cout << "Matches at positions: ";
    if (matches.empty()) {
        std::cout << "(none)";
    } else {
        for (int i = 0; i < (int)matches.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << matches[i];
        }
    }
    std::cout << "\n\n";

    // Test 4: Single character
    std::string text4 = "mississippi";
    std::string pattern4 = "s";

    std::cout << "Text:    \"" << text4 << "\"\n";
    std::cout << "Pattern: \"" << pattern4 << "\"\n";

    matches = rabin_karp_search(text4, pattern4);
    std::cout << "Matches at positions: ";
    for (int i = 0; i < (int)matches.size(); i++) {
        if (i > 0) std::cout << ", ";
        std::cout << matches[i];
    }
    std::cout << "\n";

    // Test 5: Fingerprint demo
    std::cout << "\nFingerprint examples:\n";
    const long long BASE = 256;
    const long long MOD = 1000000007LL;
    std::string s1 = "hello";
    std::string s2 = "world";
    std::string s3 = "hello";
    std::cout << "  f(\"hello\") = " << string_fingerprint(s1, BASE, MOD) << "\n";
    std::cout << "  f(\"world\") = " << string_fingerprint(s2, BASE, MOD) << "\n";
    std::cout << "  f(\"hello\") = " << string_fingerprint(s3, BASE, MOD) << " (same as above)\n";
}

} // namespace chapter8
