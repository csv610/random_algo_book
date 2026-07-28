#pragma once

#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <cmath>
#include <random>
#include <climits>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <iomanip>
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Count-Min Sketch (Cormode & Muthukrishnan, 2005)
// Probabilistic data structure for frequency estimation in data streams.
// Uses O(w * d) memory where w = ceil(e/epsilon), d = ceil(ln(1/delta)).
// Guarantees:
//   - Estimated frequency >= true frequency (never underestimates)
//   - Estimated frequency <= true frequency + epsilon * N
//     with probability >= 1 - delta
// ---------------------------------------------------------------------------

class StreamingCountMinSketch {
public:
    // width = ceil(e/eps), depth = ceil(ln(1/delta))
    // eps = error tolerance, delta = failure probability
    StreamingCountMinSketch(double eps, double delta, std::mt19937& rng)
        : width_(static_cast<int>(std::ceil(2.71828 / eps))),
          depth_(static_cast<int>(std::ceil(std::log(1.0 / delta)))),
          total_count_(0) {

        table_.assign(depth_, std::vector<long long>(width_, 0));
        seeds_.resize(depth_);
        std::uniform_int_distribution<unsigned> dist;
        for (int i = 0; i < depth_; ++i) {
            seeds_[i] = dist(rng);
        }
    }

    void update(const std::string& item, long long count = 1) {
        for (int i = 0; i < depth_; ++i) {
            int idx = hash(item, seeds_[i]) % width_;
            table_[i][idx] += count;
        }
        total_count_ += count;
    }

    long long estimate(const std::string& item) const {
        long long min_val = LLONG_MAX;
        for (int i = 0; i < depth_; ++i) {
            int idx = hash(item, seeds_[i]) % width_;
            min_val = std::min(min_val, table_[i][idx]);
        }
        return min_val;
    }

    // Estimate frequency of all items that were never explicitly updated
    long long estimate_zero() const {
        // Minimum value in any row gives an upper bound on noise
        long long min_row = LLONG_MAX;
        for (int i = 0; i < depth_; ++i) {
            long long row_min = LLONG_MAX;
            for (int j = 0; j < width_; ++j) {
                row_min = std::min(row_min, table_[i][j]);
            }
            min_row = std::min(min_row, row_min);
        }
        return min_row;
    }

    // Top-k heavy hitters via the "space-saving" technique on the sketch
    struct FreqItem {
        std::string item;
        long long estimated_count;
    };

    int width() const { return width_; }
    int depth() const { return depth_; }
    long long total_count() const { return total_count_; }

private:
    int width_;
    int depth_;
    std::vector<std::vector<long long>> table_;
    std::vector<unsigned> seeds_;
    long long total_count_;

    static unsigned hash(const std::string& item, unsigned seed) {
        // FNV-1a hash
        unsigned h = seed ^ 2166136261u;
        for (char c : item) {
            h ^= static_cast<unsigned>(c);
            h *= 16777619u;
        }
        return h;
    }
};

// ---------------------------------------------------------------------------
// Flajolet-Martin Algorithm (1984)
// Estimates the number of distinct elements in a data stream.
// Uses O(log m) bits per register where m = universe size.
// Expected relative error: O(1/sqrt(p)) where p = number of registers.
//
// Key idea: hash each element to {0,1}^64, look at position of least
// significant 1-bit. The maximum position seen across all elements gives
// an estimate of log2(distinct_count).
// ---------------------------------------------------------------------------

class FlajoletMartin {
public:
    explicit FlajoletMartin(int num_registers, std::mt19937& rng)
        : registers_(num_registers, 0), seeds_(num_registers) {
        std::uniform_int_distribution<unsigned long long> dist;
        for (int i = 0; i < num_registers; ++i) {
            seeds_[i] = static_cast<uint64_t>(dist(rng));
        }
    }

    void update(const std::string& item) {
        for (int i = 0; i < static_cast<int>(registers_.size()); ++i) {
            uint64_t h = hash64(item, seeds_[i]);
            int pos = least_significant_bit(h);
            registers_[i] = std::max(registers_[i], static_cast<uint64_t>(pos));
        }
        count_++;
    }

    // Estimate number of distinct elements
    // Uses group-wise averaging for variance reduction
    double estimate() const {
        int p = static_cast<int>(registers_.size());
        int groups = std::max(1, p / 32);
        int group_size = p / groups;

        std::vector<double> group_estimates;
        for (int g = 0; g < groups; ++g) {
            double sum = 0.0;
            for (int j = 0; j < group_size; ++j) {
                int idx = g * group_size + j;
                sum += static_cast<double>(registers_[idx]);
            }
            double avg = sum / group_size;
            double phi = 0.77351113; // correction constant
            group_estimates.push_back(std::pow(2.0, avg + phi));
        }

        // Return median of group estimates
        std::sort(group_estimates.begin(), group_estimates.end());
        return group_estimates[groups / 2];
    }

    long long stream_count() const { return count_; }

private:
    std::vector<uint64_t> registers_;
    std::vector<uint64_t> seeds_;
    long long count_ = 0;

    static uint64_t hash64(const std::string& item, uint64_t seed) {
        // SplitMix64-style hash
        uint64_t h = seed;
        for (char c : item) {
            h ^= static_cast<uint64_t>(c);
            h ^= h >> 33;
            h *= 0xff51afd7ed558ccdULL;
            h ^= h >> 33;
            h *= 0xc4ceb9fe1a85ec53ULL;
            h ^= h >> 33;
        }
        return h;
    }

    static int least_significant_bit(uint64_t x) {
        if (x == 0) return 64;
        int pos = 0;
        while ((x & 1) == 0) {
            x >>= 1;
            pos++;
        }
        return pos;
    }
};

// ---------------------------------------------------------------------------
// Misra-Gries Heavy Hitters (1982)
// Finds all items with frequency > n/k in O(n) time and O(k) space.
// Guarantees:
//   - All items with frequency > n/k are reported
//   - No item with frequency < n/k - n/(k+1) is missed
// ---------------------------------------------------------------------------

class MisraGries {
public:
    explicit MisraGries(int k) : k_(k) {}

    void update(const std::string& item) {
        if (counters_.count(item)) {
            counters_[item]++;
        } else if (static_cast<int>(counters_.size()) < k_ - 1) {
            counters_[item] = 1;
        } else {
            // Decrement all counters, remove zeros
            std::vector<std::string> to_remove;
            for (auto& [key, val] : counters_) {
                val--;
                if (val == 0) to_remove.push_back(key);
            }
            for (auto& key : to_remove) {
                counters_.erase(key);
            }
        }
        total_count_++;
    }

    struct FreqItem {
        std::string item;
        long long estimated_count;
    };

    // Get candidates with their exact counts
    // (must be verified against the stream for exact frequencies)
    std::vector<FreqItem> candidates() const {
        std::vector<FreqItem> result;
        for (auto& [item, count] : counters_) {
            result.push_back({item, count});
        }
        std::sort(result.begin(), result.end(),
                  [](const FreqItem& a, const FreqItem& b) {
                      return a.estimated_count > b.estimated_count;
                  });
        return result;
    }

    // Returns estimated frequency (lower bound) for an item
    long long estimate(const std::string& item) const {
        auto it = counters_.find(item);
        if (it != counters_.end()) return it->second;
        return 0;
    }

    int k() const { return k_; }
    long long total_count() const { return total_count_; }

private:
    int k_;
    std::unordered_map<std::string, long long> counters_;
    long long total_count_ = 0;
};

// ---------------------------------------------------------------------------
// Demonstration: Streaming Analytics for Network Traffic
// ---------------------------------------------------------------------------
inline void demonstrate_streaming_analytics() {
    std::mt19937 rng(42);

    println("=== Streaming Analytics: Real-time Network Traffic Analysis ===\n");

    // Simulate network traffic: source IPs and packet counts
    std::vector<std::pair<std::string, int>> traffic;
    std::vector<std::string> top_talkers = {"10.0.0.1", "10.0.0.2", "10.0.0.3"};
    std::uniform_int_distribution<int> port_dist(1, 100);
    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);

    for (int i = 0; i < 100000; ++i) {
        std::string src;
        if (prob_dist(rng) < 0.4) {
            // 40% of traffic from top 3 talkers
            src = top_talkers[prob_dist(rng) * 3];
        } else if (prob_dist(rng) < 0.1) {
            // 10% from a set of 50 moderate sources
            int host = 100 + static_cast<int>(prob_dist(rng) * 50);
            src = "10.0." + std::to_string(host / 256) + "." + std::to_string(host % 256);
        } else {
            // 50% from long tail of random sources
            int host = 1000 + static_cast<int>(prob_dist(rng) * 10000);
            src = "192.168." + std::to_string(host / 256) + "." + std::to_string(host % 256);
        }
        traffic.push_back({src, port_dist(rng)});
    }

    // --- 1. Count-Min Sketch: frequency estimation ---
    println("--- 1. Count-Min Sketch (eps=0.001, delta=0.01) ---");
    {
        StreamingCountMinSketch cms(0.001, 0.01, rng);

        for (auto& [ip, port] : traffic) {
            cms.update(ip);
        }

        println("  Memory: {} x {} = {} cells (vs {} for exact count)",
                      cms.depth(), cms.width(), cms.depth() * cms.width(),
                      100000);

        println("  Top talkers (estimated frequency):");
        for (auto& ip : top_talkers) {
            long long est = cms.estimate(ip);
            println("    {}: {} packets", ip, est);
        }

        // Check accuracy
        std::unordered_map<std::string, long long> exact;
        for (auto& [ip, port] : traffic) exact[ip]++;
        println("  Exact vs estimated for top talkers:");
        for (auto& ip : top_talkers) {
            println("    {}: exact={}, est={}, error={}",
                          ip, exact[ip], cms.estimate(ip),
                          cms.estimate(ip) - exact[ip]);
        }
    }

    // --- 2. Flajolet-Martin: distinct element counting ---
    println("\n--- 2. Flajolet-Martin Distinct Count Estimation ---");
    {
        FlajoletMartin fm(64, rng);
        for (auto& [ip, port] : traffic) {
            fm.update(ip);
        }

        // Count exact distinct
        std::unordered_set<std::string> distinct;
        for (auto& [ip, port] : traffic) distinct.insert(ip);

        println("  Stream length: {}", fm.stream_count());
        println("  Estimated distinct elements: {:.0f}", fm.estimate());
        println("  Exact distinct elements: {}", distinct.size());
        println("  Relative error: {:.2f}%",
                      100.0 * std::abs(fm.estimate() - (double)distinct.size()) / distinct.size());
    }

    // --- 3. Misra-Gries: heavy hitter detection ---
    println("\n--- 3. Misra-Gries Heavy Hitter Detection (k=10) ---");
    {
        MisraGries mg(10);

        for (auto& [ip, port] : traffic) {
            mg.update(ip);
        }

        auto candidates = mg.candidates();
        println("  Candidates found: {}", candidates.size());
        println("  Stream length: {}", mg.total_count());
        println("  Threshold n/k = {}", mg.total_count() / mg.k());

        // Verify with exact counts
        std::unordered_map<std::string, long long> exact;
        for (auto& [ip, port] : traffic) exact[ip]++;

        println("  Candidate  Estimated  Exact");
        for (auto& c : candidates) {
            long long ex = exact[c.item];
            println("    {:<15} {:>8} {:>8}{}",
                          c.item, c.estimated_count, ex,
                          ex > mg.total_count() / mg.k() ? "  *** TRUE HEAVY HITTER" : "");
        }
    }
}

} // namespace ral
