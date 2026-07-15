#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <list>
#include <functional>
#include <iomanip>
#include <cassert>
#include <cmath>

namespace chapter8 {

// Mersenne prime 2^31 - 1, used as modulus in hash functions
static constexpr long long HASH_PRIME = 2147483647LL; // 2^31 - 1

class UniversalHashTable {
private:
    int table_size_;
    long long a_, b_;
    std::vector<std::list<int>> table_;
    int count_;

    long long hash(long long x) const {
        return ((a_ * x + b_) % HASH_PRIME) % table_size_;
    }

public:
    explicit UniversalHashTable(int size, std::mt19937& rng)
        : table_size_(size), count_(0) {
        std::uniform_int_distribution<long long> a_dist(1, HASH_PRIME - 1);
        std::uniform_int_distribution<long long> b_dist(0, HASH_PRIME - 1);
        a_ = a_dist(rng);
        b_ = b_dist(rng);
        table_.resize(table_size_);
    }

    void insert(int key) {
        long long idx = hash(key);
        for (int v : table_[idx]) {
            if (v == key) return;
        }
        table_[idx].push_back(key);
        count_++;
    }

    bool search(int key) const {
        long long idx = hash(key);
        for (int v : table_[idx]) {
            if (v == key) return true;
        }
        return false;
    }

    void remove(int key) {
        long long idx = hash(key);
        auto& chain = table_[idx];
        for (auto it = chain.begin(); it != chain.end(); ++it) {
            if (*it == key) {
                chain.erase(it);
                count_--;
                return;
            }
        }
    }

    int size() const { return count_; }
    int table_size() const { return table_size_; }

    double load_factor() const {
        return static_cast<double>(count_) / table_size_;
    }

    double avg_chain_length() const {
        int total = 0;
        for (const auto& chain : table_) {
            total += (int)chain.size();
        }
        return static_cast<double>(total) / table_size_;
    }

    int max_chain_length() const {
        int mx = 0;
        for (const auto& chain : table_) {
            mx = std::max(mx, (int)chain.size());
        }
        return mx;
    }
};

// Fredman-Komlos-Szemeredi scheme: O(n) space with O(1) worst-case lookup
class PerfectHashTable {
private:
    struct SecondaryTable {
        long long a, b;
        std::vector<int> slots;
        std::vector<bool> occupied;
        int n;
        SecondaryTable() : a(0), b(0), n(0) {}
    };

    int primary_size_;
    std::vector<int> primary_keys_;
    std::vector<std::vector<int>> buckets_;
    std::vector<SecondaryTable> secondary_;
    long long primary_a_, primary_b_;
    std::mt19937& rng_;
    int count_;

    long long primary_hash(long long x) const {
        return ((primary_a_ * x + primary_b_) % HASH_PRIME) % primary_size_;
    }

    void build_secondary(int bucket_id, const std::vector<int>& keys, std::mt19937& rng) {
        auto& st = secondary_[bucket_id];
        st.n = (int)keys.size();
        if (st.n == 0) {
            st.slots.clear();
            st.occupied.clear();
            return;
        }

        int table_size = st.n * st.n;
        if (table_size < 1) table_size = 1;

        std::uniform_int_distribution<long long> a_dist(1, HASH_PRIME - 1);
        std::uniform_int_distribution<long long> b_dist(0, HASH_PRIME - 1);

        bool collision_free = false;
        int attempts = 0;
        while (!collision_free && attempts < 100) {
            st.a = a_dist(rng);
            st.b = b_dist(rng);
            st.slots.assign(table_size, 0);
            st.occupied.assign(table_size, false);
            collision_free = true;

            for (int key : keys) {
                long long h = ((st.a * key + st.b) % HASH_PRIME) % table_size;
                if (st.occupied[h]) {
                    collision_free = false;
                    break;
                }
                st.occupied[h] = true;
                st.slots[h] = key;
            }
            attempts++;
        }

        if (!collision_free) {
            st.slots.assign(table_size, 0);
            st.occupied.assign(table_size, false);
            for (int key : keys) {
                long long h = ((st.a * key + st.b) % HASH_PRIME) % table_size;
                while (st.occupied[h]) {
                    h = (h + 1) % table_size;
                }
                st.occupied[h] = true;
                st.slots[h] = key;
            }
        }
    }

public:
    explicit PerfectHashTable(int n, std::mt19937& rng)
        : primary_size_(n), rng_(rng), count_(0) {
        if (primary_size_ < 1) primary_size_ = 1;
        buckets_.resize(primary_size_);
        secondary_.resize(primary_size_);

        std::uniform_int_distribution<long long> a_dist(1, HASH_PRIME - 1);
        std::uniform_int_distribution<long long> b_dist(0, HASH_PRIME - 1);
        primary_a_ = a_dist(rng);
        primary_b_ = b_dist(rng);
    }

    void insert(int key) {
        long long idx = primary_hash(key);
        buckets_[idx].push_back(key);
        count_++;

        if ((int)buckets_[idx].size() == 1 || buckets_[idx].size() % 4 == 0) {
            build_secondary((int)idx, buckets_[idx], rng_);
        }
    }

    bool search(int key) const {
        long long idx = primary_hash(key);
        const auto& st = secondary_[idx];
        if (st.n == 0) return false;
        int table_size = (int)st.slots.size();
        long long h = ((st.a * key + st.b) % HASH_PRIME) % table_size;
        return st.occupied[h] && st.slots[h] == key;
    }

    void remove(int key) {
        long long idx = primary_hash(key);
        auto& bucket = buckets_[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (*it == key) {
                bucket.erase(it);
                count_--;
                build_secondary((int)idx, bucket, rng_);
                return;
            }
        }
    }

    int size() const { return count_; }

    size_t total_space() const {
        size_t total = 0;
        for (const auto& st : secondary_) {
            total += st.slots.size();
        }
        return total;
    }
};

inline void demonstrate_hashing() {
    std::cout << "=== Hash Tables ===\n\n";

    std::mt19937 rng(42);

    // Universal hash table demo
    std::cout << "--- Universal Hash Table with Chaining ---\n";
    {
        int n = 100;
        UniversalHashTable ht(n, rng);
        for (int i = 1; i <= n; i++) {
            ht.insert(i);
        }
        std::cout << "  Inserted " << n << " elements into table of size " << n << "\n";
        std::cout << "  Load factor: " << std::fixed << std::setprecision(2) << ht.load_factor() << "\n";
        std::cout << "  Avg chain length: " << std::fixed << std::setprecision(2) << ht.avg_chain_length() << "\n";
        std::cout << "  Max chain length: " << ht.max_chain_length() << "\n";
        std::cout << "  Search 50: " << (ht.search(50) ? "found" : "not found") << "\n";
        std::cout << "  Search 999: " << (ht.search(999) ? "found" : "not found") << "\n";
        ht.remove(50);
        std::cout << "  After removing 50, search 50: " << (ht.search(50) ? "found" : "not found") << "\n";
    }

    // Universal hash table: collision analysis
    std::cout << "\n--- Collision Analysis (Universal Hashing) ---\n";
    {
        std::vector<int> sizes = {100, 500, 1000, 5000};
        std::cout << std::setw(6) << "n"
                  << std::setw(12) << "avg_chain"
                  << std::setw(12) << "max_chain"
                  << std::setw(12) << "E[max]*"
                  << "\n";
        for (int n : sizes) {
            double total_avg = 0;
            double total_max = 0;
            int trials = 1000;
            for (int t = 0; t < trials; t++) {
                std::mt19937 trial_rng(t * 77777 + n);
                UniversalHashTable ht(n, trial_rng);
                for (int i = 1; i <= n; i++) {
                    ht.insert(i);
                }
                total_avg += ht.avg_chain_length();
                total_max += ht.max_chain_length();
            }
            double expected_max = std::log(n) / std::log(std::log(n));
            std::cout << std::setw(6) << n
                      << std::setw(12) << std::fixed << std::setprecision(2) << total_avg / trials
                      << std::setw(12) << std::fixed << std::setprecision(1) << total_max / trials
                      << std::setw(12) << std::fixed << std::setprecision(1) << expected_max
                      << "\n";
        }
    }

    // Perfect hashing demo
    std::cout << "\n--- FKS Perfect Hashing ---\n";
    {
        int n = 200;
        PerfectHashTable pht(n, rng);
        for (int i = 1; i <= n; i++) {
            pht.insert(i * 3);
        }
        std::cout << "  Inserted " << n << " elements\n";
        std::cout << "  Total secondary table space: " << pht.total_space() << " (O(n)=" << n << ")\n";
        std::cout << "  Space ratio: " << std::fixed << std::setprecision(2)
                  << (double)pht.total_space() / n << "x\n";
        std::cout << "  Search 150: " << (pht.search(150) ? "found" : "not found") << "\n";
        std::cout << "  Search 300: " << (pht.search(300) ? "found" : "not found") << "\n";
        std::cout << "  Search 999: " << (pht.search(999) ? "found" : "not found") << "\n";
    }

    // Compare universal vs perfect
    std::cout << "\n--- Universal vs Perfect Hashing Comparison ---\n";
    {
        std::vector<int> sizes = {100, 500, 1000};
        std::cout << std::setw(6) << "n"
                  << std::setw(14) << "UHT_space"
                  << std::setw(14) << "PHT_space"
                  << std::setw(14) << "ratio"
                  << "\n";
        for (int n : sizes) {
            std::mt19937 rng1(12345);
            std::mt19937 rng2(12345);
            UniversalHashTable uht(n, rng1);
            PerfectHashTable pht(n, rng2);
            for (int i = 1; i <= n; i++) {
                uht.insert(i);
                pht.insert(i);
            }
            std::cout << std::setw(6) << n
                      << std::setw(14) << n << " (chains)"
                      << std::setw(14) << pht.total_space()
                      << std::setw(14) << std::fixed << std::setprecision(2)
                      << (double)pht.total_space() / n << "x"
                      << "\n";
        }
    }
}

} // namespace chapter8
