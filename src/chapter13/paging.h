#pragma once
// Chapter 13: Paging Algorithms
// Deterministic and randomized paging against an oblivious adversary.
// Competitive analysis: LRU, FIFO, Random, Marking, and OPT (Belady's).

#include <iostream>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>
#include <print>
#include <format>
#include <ranges>
#include <cmath>
#include <numeric>
#include <functional>

#include "../chapter10/random_utils.h"

namespace randalgo {

// ============================================================
// PageCache: basic cache abstraction
// ============================================================

struct PageCache {
    int capacity;
    std::unordered_set<int> pages;
    std::deque<int> order;  // recency / arrival ordering

    explicit PageCache(int k) : capacity(k) {}

    [[nodiscard]] bool contains(int page) const {
        return pages.contains(page);
    }

    void insert(int page) {
        if (pages.contains(page)) return;
        if (static_cast<int>(pages.size()) == capacity) {
            evict();
        }
        pages.insert(page);
        order.push_back(page);
    }

    void evict() {
        if (order.empty()) return;
        int victim = order.front();
        order.pop_front();
        pages.erase(victim);
    }

    void touch(int page) {
        // Move page to back of recency order
        if (!pages.contains(page)) return;
        order.erase(
            std::ranges::find(order, page));
        order.push_back(page);
    }

    void clear() {
        pages.clear();
        order.clear();
    }

    [[nodiscard]] std::vector<int> snapshot() const {
        return {order.begin(), order.end()};
    }

    [[nodiscard]] int size() const {
        return static_cast<int>(pages.size());
    }
};

// ============================================================
// Deterministic LRU (Least Recently Used)
// ============================================================

int deterministic_lru(int cache_size, const std::vector<int>& requests, bool verbose = false) {
    PageCache cache(cache_size);
    int misses = 0;

    for (int step = 0; auto page : requests) {
        ++step;
        if (cache.contains(page)) {
            cache.touch(page);
            if (verbose) {
                std::println("  Step {:2d}: page {} HIT   cache={}",
                             step, page, cache.snapshot());
            }
        } else {
            ++misses;
            cache.insert(page);
            if (verbose) {
                std::println("  Step {:2d}: page {} MISS  cache={}",
                             step, page, cache.snapshot());
            }
        }
    }
    return misses;
}

// ============================================================
// Deterministic FIFO (First In First Out)
// ============================================================

int deterministic_fifo(int cache_size, const std::vector<int>& requests, bool verbose = false) {
    PageCache cache(cache_size);
    int misses = 0;

    for (int step = 0; auto page : requests) {
        ++step;
        if (cache.contains(page)) {
            if (verbose) {
                std::println("  Step {:2d}: page {} HIT   cache={}",
                             step, page, cache.snapshot());
            }
        } else {
            ++misses;
            cache.insert(page);
            if (verbose) {
                std::println("  Step {:2d}: page {} MISS  cache={}",
                             step, page, cache.snapshot());
            }
        }
    }
    return misses;
}

// ============================================================
// Randomized Paging (oblivious adversary)
// On a miss, evict a uniformly random page from the cache.
// ============================================================

int random_paging(int cache_size, const std::vector<int>& requests, bool verbose = false) {
    PageCache cache(cache_size);
    int misses = 0;
    auto& engine = rng().engine();

    for (int step = 0; auto page : requests) {
        ++step;
        if (cache.contains(page)) {
            if (verbose) {
                std::println("  Step {:2d}: page {} HIT   cache={}",
                             step, page, cache.snapshot());
            }
        } else {
            ++misses;
            if (static_cast<int>(cache.pages.size()) == cache.capacity) {
                // Evict a random page
                std::uniform_int_distribution<int> dist(
                    0, static_cast<int>(cache.order.size()) - 1);
                int idx = dist(engine);
                int victim = cache.order[idx];
                cache.order.erase(cache.order.begin() + idx);
                cache.pages.erase(victim);
            }
            cache.pages.insert(page);
            cache.order.push_back(page);
            if (verbose) {
                std::println("  Step {:2d}: page {} MISS  cache={}",
                             step, page, cache.snapshot());
            }
        }
    }
    return misses;
}

// ============================================================
// Marking Algorithm
// On first access in a "round", mark the page.
// On miss, evict a random unmarked page.
// When all pages are marked, reset all marks and start new round.
// ============================================================

int marking_algorithm(int cache_size, const std::vector<int>& requests, bool verbose = false) {
    PageCache cache(cache_size);
    std::unordered_set<int> marked;
    int misses = 0;
    int round = 1;
    auto& engine = rng().engine();

    for (int step = 0; auto page : requests) {
        ++step;
        if (cache.contains(page)) {
            marked.insert(page);
            if (verbose) {
                std::println("  Step {:2d}: page {} HIT   cache={} marked={} round={}",
                             step, page, cache.snapshot(), marked.size(), round);
            }
        } else {
            ++misses;
            if (static_cast<int>(cache.pages.size()) == cache.capacity) {
                // Collect unmarked pages in cache
                std::vector<int> unmarked;
                for (int p : cache.pages)
                    if (!marked.contains(p))
                        unmarked.push_back(p);

                if (unmarked.empty()) {
                    // All marked: reset marks, start new round
                    marked.clear();
                    ++round;
                    // Recompute unmarked (now all are unmarked)
                    unmarked = cache.snapshot();
                }

                // Evict random unmarked page
                std::uniform_int_distribution<int> dist(
                    0, static_cast<int>(unmarked.size()) - 1);
                int victim = unmarked[dist(engine)];
                cache.order.erase(
                    std::ranges::find(cache.order, victim));
                cache.pages.erase(victim);
            }
            cache.pages.insert(page);
            cache.order.push_back(page);
            marked.insert(page);
            if (verbose) {
                std::println("  Step {:2d}: page {} MISS  cache={} marked={} round={}",
                             step, page, cache.snapshot(), marked.size(), round);
            }
        }
    }
    return misses;
}

// ============================================================
// Optimal Offline: Belady's Algorithm
// Evict the page whose next use is farthest in the future.
// Uses a set of (next_use, page) pairs for efficient lookup.
// ============================================================

int optimal_offline(int cache_size, const std::vector<int>& requests, bool verbose = false) {
    int n = static_cast<int>(requests.size());

    // Precompute next_use[i] = next index where requests[i] appears, or INT_MAX
    std::vector<int> next_use(n, INT_MAX);
    std::unordered_map<int, int> last_seen;
    for (int i = n - 1; i >= 0; --i) {
        int page = requests[i];
        if (last_seen.contains(page))
            next_use[i] = last_seen[page];
        last_seen[page] = i;
    }

    std::unordered_set<int> cache_pages;
    // set of (next_use, page) for cache contents, ordered by next_use descending
    // We use a set of pairs; the "farthest" is the one with the largest next_use.
    std::set<std::pair<int, int>> future_uses;  // (next_use, page)
    std::unordered_map<int, int> page_to_next;  // page -> its current next_use

    int misses = 0;

    for (int step = 0; step < n; ++step) {
        int page = requests[step];

        if (cache_pages.contains(page)) {
            // Update its next_use
            future_uses.erase({page_to_next[page], page});
            page_to_next[page] = next_use[step];
            future_uses.insert({next_use[step], page});
            if (verbose) {
                std::println("  Step {:2d}: page {} HIT   cache={}",
                             step + 1, page, [&]() {
                                 std::vector<int> v;
                                 for (auto& [nu, p] : future_uses) v.push_back(p);
                                 return v;
                             }());
            }
        } else {
            ++misses;
            if (static_cast<int>(cache_pages.size()) == cache_size) {
                // Evict the page with the farthest next use (or never again)
                auto [farthest_use, victim] = *future_uses.rbegin();
                future_uses.erase({farthest_use, victim});
                page_to_next.erase(victim);
                cache_pages.erase(victim);
            }
            cache_pages.insert(page);
            page_to_next[page] = next_use[step];
            future_uses.insert({next_use[step], page});
            if (verbose) {
                std::println("  Step {:2d}: page {} MISS  cache={}",
                             step + 1, page, [&]() {
                                 std::vector<int> v;
                                 for (auto& [nu, p] : future_uses) v.push_back(p);
                                 return v;
                             }());
            }
        }
    }
    return misses;
}

// ============================================================
// Competitive Ratio: run an algorithm and OPT, return ratio
// ============================================================

double compute_competitive_ratio(
    std::function<int(int, const std::vector<int>&, bool)> algorithm,
    int cache_size,
    const std::vector<int>& requests)
{
    int alg_misses = algorithm(cache_size, requests, false);
    int opt_misses = optimal_offline(cache_size, requests, false);
    if (opt_misses == 0) return 1.0;
    return static_cast<double>(alg_misses) / static_cast<double>(opt_misses);
}

// ============================================================
// Random sequence generator
// ============================================================

std::vector<int> generate_random_requests(int length, int num_pages) {
    std::vector<int> requests(length);
    auto& engine = rng().engine();
    std::uniform_int_distribution<int> dist(1, num_pages);
    for (auto& r : requests) r = dist(engine);
    return requests;
}

// ============================================================
// Demonstration
// ============================================================

void demonstrate_paging() {
    std::println("=== Paging Algorithms (Chapter 13) ===\n");

    // ---- Small example with verbose output ----
    std::println("--- Small Example: Cache size k=3 ---");
    std::println("Request sequence: {{1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5}}\n");

    std::vector<int> requests = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    int k = 3;

    std::println("[LRU]");
    int lru_misses = deterministic_lru(k, requests, true);
    std::println("  Total misses: {}\n", lru_misses);

    std::println("[FIFO]");
    int fifo_misses = deterministic_fifo(k, requests, true);
    std::println("  Total misses: {}\n", fifo_misses);

    std::println("[Random Paging]");
    int rand_misses = random_paging(k, requests, true);
    std::println("  Total misses: {}\n", rand_misses);

    std::println("[Marking Algorithm]");
    int mark_misses = marking_algorithm(k, requests, true);
    std::println("  Total misses: {}\n", mark_misses);

    std::println("[OPT (Belady's Offline)]");
    int opt_misses = optimal_offline(k, requests, true);
    std::println("  Total misses: {}\n", opt_misses);

    // ---- Summary of small example ----
    std::println("--- Small Example Summary ---");
    std::println("  LRU:    {:2d} misses  ratio={:.2f}", lru_misses,
                 static_cast<double>(lru_misses) / opt_misses);
    std::println("  FIFO:   {:2d} misses  ratio={:.2f}", fifo_misses,
                 static_cast<double>(fifo_misses) / opt_misses);
    std::println("  Random: {:2d} misses  ratio={:.2f}", rand_misses,
                 static_cast<double>(rand_misses) / opt_misses);
    std::println("  Mark:   {:2d} misses  ratio={:.2f}", mark_misses,
                 static_cast<double>(mark_misses) / opt_misses);
    std::println("  OPT:    {:2d} misses", opt_misses);
    std::println("");

    // ---- Competitive ratio: adversarial sequence ----
    std::println("--- Competitive Ratio: Adversarial Sequences ---");
    std::println("For k-page cache, an adversary can force k-regret.");
    std::println("LRU and FIFO are k-competitive but NOT (k-1)-competitive.\n");

    // Construct adversary's preferred sequence for k=3
    // Cyclic sequence of 4 pages: forces LRU/FIFO to miss every other request
    std::vector<int> adseq;
    std::vector<int> cycle = {1, 2, 3, 4};
    for (int i = 0; i < 20; ++i)
        adseq.push_back(cycle[i % 4]);

    double lru_ratio = compute_competitive_ratio(deterministic_lru, k, adseq);
    double fifo_ratio = compute_competitive_ratio(deterministic_fifo, k, adseq);
    double rnd_ratio = compute_competitive_ratio(random_paging, k, adseq);
    double mark_ratio = compute_competitive_ratio(marking_algorithm, k, adseq);

    std::println("  Adversarial sequence (cycle of 4, length 20, k=3):");
    std::println("  LRU ratio:    {:.2f}", lru_ratio);
    std::println("  FIFO ratio:   {:.2f}", fifo_ratio);
    std::println("  Random ratio: {:.2f}", rnd_ratio);
    std::println("  Mark ratio:   {:.2f}", mark_ratio);
    std::println("");

    // ---- Average-case on random sequences ----
    std::println("--- Average Case: Random Sequences ---");
    constexpr int num_trials = 100;
    constexpr int seq_len = 1000;
    constexpr int num_pages = 10;

    for (int cache_k : {2, 3, 5}) {
        double total_lru = 0, total_fifo = 0, total_rand = 0, total_mark = 0, total_opt = 0;

        for ([[maybe_unused]] int _ : std::views::iota(0, num_trials)) {
            auto req = generate_random_requests(seq_len, num_pages);
            total_lru  += deterministic_lru(cache_k, req, false);
            total_fifo += deterministic_fifo(cache_k, req, false);
            total_rand += random_paging(cache_k, req, false);
            total_mark += marking_algorithm(cache_k, req, false);
            total_opt  += optimal_offline(cache_k, req, false);
        }

        double avg_lru  = total_lru  / num_trials;
        double avg_fifo = total_fifo / num_trials;
        double avg_rand = total_rand / num_trials;
        double avg_mark = total_mark / num_trials;
        double avg_opt  = total_opt  / num_trials;

        std::println("  k={}: seq_len={} num_pages={} ({} trials)", cache_k, seq_len, num_pages, num_trials);
        std::println("    LRU:    {:.1f} avg misses  (ratio {:.2f})", avg_lru, avg_lru / avg_opt);
        std::println("    FIFO:   {:.1f} avg misses  (ratio {:.2f})", avg_fifo, avg_fifo / avg_opt);
        std::println("    Random: {:.1f} avg misses  (ratio {:.2f})", avg_rand, avg_rand / avg_opt);
        std::println("    Mark:   {:.1f} avg misses  (ratio {:.2f})", avg_mark, avg_mark / avg_opt);
        std::println("    OPT:    {:.1f} avg misses", avg_opt);
        std::println("");
    }

    // ---- Scalability test ----
    std::println("--- Scalability: Sequence Length vs Misses ---");
    {
        int cache_k = 3;
        std::println("  Cache size k={}, {} pages, 50 trials per length\n",
                     cache_k, num_pages);

        std::println("  {:>10s}  {:>8s}  {:>8s}  {:>8s}  {:>8s}  {:>8s}",
                     "Length", "LRU", "FIFO", "Random", "Mark", "OPT");

        for (int len : {100, 500, 1000, 5000, 10000}) {
            double tl = 0, tf = 0, tr = 0, tm = 0, to = 0;
            constexpr int trials = 50;
            for ([[maybe_unused]] int _ : std::views::iota(0, trials)) {
                auto req = generate_random_requests(len, num_pages);
                tl += deterministic_lru(cache_k, req, false);
                tf += deterministic_fifo(cache_k, req, false);
                tr += random_paging(cache_k, req, false);
                tm += marking_algorithm(cache_k, req, false);
                to += optimal_offline(cache_k, req, false);
            }
            std::println("  {:>10d}  {:>8.1f}  {:>8.1f}  {:>8.1f}  {:>8.1f}  {:>8.1f}",
                         len, tl / trials, tf / trials, tr / trials,
                         tm / trials, to / trials);
        }
        std::println("");
    }

    std::println("--- Theoretical Results ---");
    std::println("  LRU:    k-competitive (optimal among deterministic algorithms)");
    std::println("  FIFO:   k-competitive");
    std::println("  Random: H_k-competitive (expected), ~ ln(k) for large k");
    std::println("  Marking: H_k-competitive (expected), randomized");
    std::println("  OPT:    optimal offline (cost = lower bound)\n");
}

} // namespace randalgo
