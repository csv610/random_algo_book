#pragma once

#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <random>
#include "random_utils.h"

namespace ral {

// ============================================================
// 1. MinHash: Jaccard Similarity Estimation
// ============================================================
class MinHash {
private:
    int num_hashes_;
    std::vector<uint64_t> hash_a_;
    std::vector<uint64_t> hash_b_;
    static constexpr uint64_t PRIME = 4294967291ULL; // Large prime < 2^32

public:
    MinHash(int num_hashes, std::mt19937& rng) : num_hashes_(num_hashes) {
        std::uniform_int_distribution<uint64_t> a_dist(1, PRIME - 1);
        std::uniform_int_distribution<uint64_t> b_dist(0, PRIME - 1);
        
        for (int i = 0; i < num_hashes; ++i) {
            hash_a_.push_back(a_dist(rng));
            hash_b_.push_back(b_dist(rng));
        }
    }

    // Compute MinHash signature of a set of elements
    std::vector<uint32_t> compute_signature(const std::unordered_set<int>& set) const {
        std::vector<uint32_t> signature(num_hashes_, std::numeric_limits<uint32_t>::max());
        if (set.empty()) return signature;

        for (int i = 0; i < num_hashes_; ++i) {
            uint64_t a = hash_a_[i];
            uint64_t b = hash_b_[i];
            uint64_t min_val = std::numeric_limits<uint64_t>::max();
            
            for (int val : set) {
                // Ensure non-negative hash input by casting to uint64_t
                uint64_t x = static_cast<uint64_t>(val);
                uint64_t hash_val = ((a * x + b) % PRIME);
                if (hash_val < min_val) {
                    min_val = hash_val;
                }
            }
            signature[i] = static_cast<uint32_t>(min_val);
        }
        return signature;
    }

    // Estimate Jaccard similarity between two signatures
    static double estimate_similarity(const std::vector<uint32_t>& sig1, const std::vector<uint32_t>& sig2) {
        if (sig1.size() != sig2.size()) {
            throw std::invalid_argument("Signatures must have identical dimensions.");
        }
        int matches = 0;
        for (size_t i = 0; i < sig1.size(); ++i) {
            if (sig1[i] == sig2[i]) {
                matches++;
            }
        }
        return static_cast<double>(matches) / sig1.size();
    }
};

// ============================================================
// 2. Count-Min Sketch: Frequency Estimation
// ============================================================
class CountMinSketch {
private:
    int depth_;
    int width_;
    std::vector<std::vector<int>> table_;
    std::vector<uint64_t> hash_a_;
    std::vector<uint64_t> hash_b_;
    static constexpr uint64_t PRIME = 4294967291ULL;

    int hash(int item, int depth_idx) const {
        uint64_t x = static_cast<uint64_t>(item);
        return static_cast<int>(((hash_a_[depth_idx] * x + hash_b_[depth_idx]) % PRIME) % width_);
    }

public:
    // Create sketch with width w (precision control) and depth d (probability control)
    CountMinSketch(int width, int depth, std::mt19937& rng) 
        : depth_(depth), width_(width), table_(depth, std::vector<int>(width, 0)) {
        
        std::uniform_int_distribution<uint64_t> a_dist(1, PRIME - 1);
        std::uniform_int_distribution<uint64_t> b_dist(0, PRIME - 1);
        
        for (int i = 0; i < depth; ++i) {
            hash_a_.push_back(a_dist(rng));
            hash_b_.push_back(b_dist(rng));
        }
    }

    // Add value to the item frequency
    void add(int item, int count = 1) {
        for (int i = 0; i < depth_; ++i) {
            int bucket = hash(item, i);
            table_[i][bucket] += count;
        }
    }

    // Retrieve conservative estimate of frequency
    int estimate(int item) const {
        int min_val = std::numeric_limits<int>::max();
        for (int i = 0; i < depth_; ++i) {
            int bucket = hash(item, i);
            min_val = std::min(min_val, table_[i][bucket]);
        }
        return min_val;
    }
};

// ============================================================
// 3. HyperLogLog: Cardinality Estimation
// ============================================================
class HyperLogLog {
private:
    int b_; // precision bits (defines register count m = 2^b)
    int m_;
    double alpha_m_;
    std::vector<uint8_t> registers_;

    // MurmurHash3 64-bit mixer for inputs
    static uint64_t hash64(uint64_t key) {
        key ^= key >> 33;
        key *= 0xff51afd7ed558ccdULL;
        key ^= key >> 33;
        key *= 0xc4ceb9fe1a85ec53ULL;
        key ^= key >> 33;
        return key;
    }

    static int get_leading_zeros(uint64_t val, int max_bits) {
        if (val == 0) return max_bits;
        return std::countl_zero(val) + 1;
    }

public:
    explicit HyperLogLog(int b) : b_(b), m_(1 << b), registers_(m_, 0) {
        if (b < 4 || b > 16) {
            throw std::invalid_argument("Precision bit b must be between 4 and 16.");
        }
        
        // Constant alpha estimation
        if (m_ == 16) alpha_m_ = 0.673;
        else if (m_ == 32) alpha_m_ = 0.697;
        else if (m_ == 64) alpha_m_ = 0.709;
        else alpha_m_ = 0.7213 / (1.0 + 1.079 / m_);
    }

    // Insert an item into the HLL sketch
    void insert(int item) {
        uint64_t raw_hash = hash64(static_cast<uint64_t>(item));
        
        // Use first b bits as register index
        uint64_t idx = raw_hash >> (64 - b_);
        
        // Find leading zeros in the remaining 64-b bits
        uint64_t rest = (raw_hash << b_) | (1ULL << (b_ - 1)); // append a 1 at end to prevent total zeros
        int zeros = get_leading_zeros(rest, 64 - b_);
        
        registers_[idx] = std::max(registers_[idx], static_cast<uint8_t>(zeros));
    }

    // Estimate cardinality of stream
    double estimate() const {
        double sum = 0.0;
        for (int i = 0; i < m_; ++i) {
            sum += std::pow(2.0, -static_cast<double>(registers_[i]));
        }
        
        double estimate = alpha_m_ * m_ * m_ / sum;
        
        // Small range correction (Linear Counting)
        if (estimate <= 2.5 * m_) {
            int zero_registers = 0;
            for (int i = 0; i < m_; ++i) {
                if (registers_[i] == 0) zero_registers++;
            }
            if (zero_registers > 0) {
                estimate = m_ * std::log(static_cast<double>(m_) / zero_registers);
            }
        }
        
        return estimate;
    }
};

} // namespace ral
