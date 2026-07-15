#pragma once
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <latch>
#include <barrier>
#include <random>
#include <print>
#include <format>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <chrono>
#include <sstream>
#include <string>

namespace randalgo {

// ---------------------------------------------------------------------------
// Simplified PRAM (Parallel Random Access Machine) simulation
// ---------------------------------------------------------------------------

class PRAM {
public:
    int num_processors;
    std::vector<long long> global_memory;
    std::vector<std::vector<long long>> local_memory;
    mutable std::mt19937 rng{std::random_device{}()};

    explicit PRAM(int p, int mem_size = 0)
        : num_processors(p),
          global_memory(mem_size, 0),
          local_memory(p, std::vector<long long>(16, 0)) {}

    // Execute a parallel step: every processor i runs fn(i, global_memory) and
    // the results are gathered into a per-processor vector.
    std::vector<long long> execute_step(
        std::function<long long(int, std::vector<long long>&)> fn) {

        std::vector<long long> results(num_processors);
        std::vector<std::jthread> threads;
        threads.reserve(num_processors);

        for (int i = 0; i < num_processors; ++i) {
            threads.emplace_back([this, i, &fn, &results]() {
                results[i] = fn(i, this->global_memory);
            });
        }
        // jthreads join automatically
        return results;
    }

    // Parallel prefix sum (Hillis-Steel algorithm) using p processors.
    // Input length n is padded to num_processors.
    std::vector<long long> prefix_sum_parallel(const std::vector<long long>& input) {
        int n = static_cast<int>(input.size());
        int p = num_processors;

        std::vector<long long> x(p, 0);
        for (int i = 0; i < std::min(n, p); ++i)
            x[i] = input[i];

        std::println("\n=== Parallel Prefix Sum (Hillis-Steel) ===");
        std::println("Input ({} elements):", n);
        print_vector(input);

        int steps = static_cast<int>(std::ceil(std::log2(p)));
        for (int d = 0; d < steps; ++d) {
            int stride = 1 << d;
            std::println("\nStep {}/{}  (stride = {}):", d + 1, steps, stride);

            std::vector<long long> old_x = x;
            auto results = execute_step([&](int i, std::vector<long long>&) -> long long {
                if (i < p && i >= stride) {
                    return old_x[i] + old_x[i - stride];
                }
                return old_x[i];
            });
            x = results;
            print_vector(x);
        }

        std::println("Prefix sum result:");
        print_vector(x);
        return x;
    }

    // Parallel list ranking.
    // Given a next-pointer array (next[i] = index of successor, -1 for tail),
    // compute the distance from each node to the tail in parallel.
    std::vector<long long> list_ranking_parallel(const std::vector<int>& next_ptr) {
        int n = static_cast<int>(next_ptr.size());

        std::println("\n=== Parallel List Ranking ===");
        std::println("List of {} nodes:", n);

        // Distance starts at 1 for every node with a successor, 0 for tail
        std::vector<long long> rank(n, 0);
        std::vector<int> next_cur = next_ptr;

        for (int i = 0; i < n; ++i) {
            if (next_cur[i] != -1) rank[i] = 1;
        }

        print_list(next_cur, rank);

        // Parallel pointer jumping: repeat until all ranks are final.
        // In the PRAM model this takes O(log n) steps.
        int max_steps = static_cast<int>(std::ceil(std::log2(n + 1)));
        for (int step = 0; step < max_steps; ++step) {
            std::vector<long long> old_rank = rank;
            std::vector<int> old_next = next_cur;
            bool any_active = false;

            auto rank_results = execute_step([&](int i, std::vector<long long>&) -> long long {
                if (i < n && old_next[i] != -1) {
                    return old_rank[i] + old_rank[old_next[i]];
                }
                return old_rank[i];
            });

            auto next_results = execute_step([&](int i, std::vector<long long>&) -> long long {
                if (i < n && old_next[i] != -1) {
                    return static_cast<long long>(old_next[old_next[i]]);
                }
                return static_cast<long long>(old_next[i]);
            });

            for (int i = 0; i < n; ++i) {
                rank[i] = rank_results[i];
                int new_next = static_cast<int>(next_results[i]);
                if (new_next != next_cur[i]) any_active = true;
                next_cur[i] = new_next;
            }

            std::println("Step {}: ranks = ", step + 1);
            print_list(next_cur, rank);
            if (!any_active) break;
        }

        std::println("Final list ranks:");
        for (int i = 0; i < n; ++i) {
            std::println("  node {} -> rank {}", i, rank[i]);
        }
        return rank;
    }

    // Parallel connected components using label propagation.
    // Each vertex starts with its own label; in each round every vertex
    // adopts the minimum label among itself and its neighbours.
    std::vector<int> connected_components_parallel(
        const std::vector<std::vector<int>>& adj) {

        int n = static_cast<int>(adj.size());
        std::println("\n=== Parallel Connected Components ===");
        std::println("Graph with {} vertices:", n);
        for (int i = 0; i < n; ++i) {
            if (!adj[i].empty())
                std::println("  {} -> [{}]", i, fmt_join(adj[i]));
        }

        std::vector<int> label(n);
        std::iota(label.begin(), label.end(), 0);

        int max_rounds = static_cast<int>(std::ceil(std::log2(n + 1)));
        for (int round = 0; round < max_rounds; ++round) {
            std::vector<int> old_label = label;
            bool changed = false;

            auto results = execute_step([&](int i, std::vector<long long>&) -> long long {
                if (i >= n) return 0;
                int mn = old_label[i];
                for (int nb : adj[i]) mn = std::min(mn, old_label[nb]);
                return static_cast<long long>(mn);
            });

            for (int i = 0; i < n; ++i) {
                label[i] = static_cast<int>(results[i]);
                if (label[i] != old_label[i]) changed = true;
            }

            std::println("Round {}: labels = [{}]", round + 1, fmt_join_int(label));
            if (!changed) break;
        }

        // Count components
        std::vector<int> unique_labels = label;
        std::sort(unique_labels.begin(), unique_labels.end());
        unique_labels.erase(std::unique(unique_labels.begin(), unique_labels.end()),
                            unique_labels.end());
        std::println("Found {} connected component(s)", unique_labels.size());
        return label;
    }

    // Sequential prefix sum for comparison
    static std::vector<long long> prefix_sum_sequential(const std::vector<long long>& input) {
        std::vector<long long> out = input;
        for (size_t i = 1; i < out.size(); ++i)
            out[i] += out[i - 1];
        return out;
    }

private:
    void print_vector(const std::vector<long long>& v) const {
        std::print("  [");
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) std::print(", ");
            std::print("{}", v[i]);
        }
        std::println("]");
    }

    void print_list(const std::vector<int>& next, const std::vector<long long>& rank) const {
        for (size_t i = 0; i < next.size(); ++i) {
            std::print("  node {} -> next={}, rank={}", i,
                       next[i] == -1 ? std::string("NULL") : std::to_string(next[i]),
                       rank[i]);
            std::println();
        }
    }

    static std::string fmt_join(const std::vector<int>& v) {
        std::ostringstream oss;
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) oss << ", ";
            oss << v[i];
        }
        return oss.str();
    }

    static std::string fmt_join_int(const std::vector<int>& v) {
        return fmt_join(v);
    }
};

// ---------------------------------------------------------------------------
// Demonstration
// ---------------------------------------------------------------------------

inline void demonstrate_pram() {
    std::println("+==============================================================+");
    std::println("|   Chapter 12: PRAM Simulation -- Parallel Algorithms        |");
    std::println("+==============================================================+");

    // --- Parallel Prefix Sum ---
    {
        int n = 16;
        std::vector<long long> input(n);
        std::iota(input.begin(), input.end(), 1);

        PRAM pram(16, n);
        auto t0 = std::chrono::steady_clock::now();
        auto parallel_result = pram.prefix_sum_parallel(input);
        auto t1 = std::chrono::steady_clock::now();

        auto t2 = std::chrono::steady_clock::now();
        auto seq_result = PRAM::prefix_sum_sequential(input);
        auto t3 = std::chrono::steady_clock::now();

        std::println("\nSequential result:");
        std::print("  [");
        for (size_t i = 0; i < seq_result.size(); ++i) {
            if (i) std::print(", ");
            std::print("{}", seq_result[i]);
        }
        std::println("]");

        bool match = (parallel_result == seq_result);
        std::println("\nResults match: {}", match ? "YES" : "NO");
        std::println("Parallel time:  {} us",
            std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count());
        std::println("Sequential time: {} us",
            std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count());
    }

    // --- Parallel List Ranking ---
    {
        // 0 -> 1 -> 2 -> 3 -> 4 (tail)
        std::vector<int> next_ptr = {1, 2, 3, 4, -1};
        PRAM pram(4, 0);
        pram.list_ranking_parallel(next_ptr);
    }

    // --- Parallel Connected Components ---
    {
        // Graph: 0-1-2, 3-4 (two components)
        std::vector<std::vector<int>> adj = {
            {1}, {0, 2}, {1}, {4}, {3}
        };
        PRAM pram(8, 0);
        auto labels = pram.connected_components_parallel(adj);
    }
}

} // namespace randalgo
