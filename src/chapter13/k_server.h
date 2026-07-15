#pragma once
// Chapter 13: The k-Server Problem
// Online algorithms for the k-server problem on metric spaces.
// Comparing deterministic greedy vs randomized against an adversary.

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <cmath>
#include <numeric>
#include <print>
#include <format>
#include <ranges>
#include <limits>
#include <optional>
#include <map>

#include "../chapter10/random_utils.h"

namespace randalgo {

// ============================================================
// ServerState: tracks server positions and accumulated cost
// ============================================================

struct ServerState {
    int k;                           // number of servers
    std::vector<int> positions;      // current positions (indices on metric space)
    long long total_cost = 0;        // total movement cost so far
    std::vector<std::pair<int,int>> movements;  // (from, to) log

    ServerState(int k, std::vector<int> initial_positions)
        : k(k), positions(std::move(initial_positions))
    {
        if (static_cast<int>(this->positions.size()) != k)
            throw std::runtime_error("ServerState: need exactly k initial positions");
    }

    void move_server(int server_idx, int new_pos, const std::vector<std::vector<int>>& dist) {
        int old_pos = positions[server_idx];
        int cost = dist[old_pos][new_pos];
        positions[server_idx] = new_pos;
        total_cost += cost;
        movements.emplace_back(old_pos, new_pos);
    }

    [[nodiscard]] bool serves(int pos) const {
        return std::ranges::find(positions, pos) != positions.end();
    }

    [[nodiscard]] std::vector<int> snapshot() const { return positions; }
};

// ============================================================
// Greedy / Deterministic: move the closest server
// ============================================================

ServerState min_server_move(ServerState state, int request,
                            const std::vector<std::vector<int>>& dist) {
    if (state.serves(request)) return state;

    int best_server = -1;
    int best_dist = std::numeric_limits<int>::max();
    for (int i = 0; i < state.k; ++i) {
        int d = dist[state.positions[i]][request];
        if (d < best_dist) {
            best_dist = d;
            best_server = i;
        }
    }
    state.move_server(best_server, request, dist);
    return state;
}

// ============================================================
// Randomized: pick a server with probability ~ 1/distance
// ============================================================

ServerState random_server_move(ServerState state, int request,
                               const std::vector<std::vector<int>>& dist) {
    if (state.serves(request)) return state;

    auto& engine = rng().engine();

    // Compute weights: w_i = 1 / (1 + dist[s_i, request])
    std::vector<double> weights(state.k);
    for (int i = 0; i < state.k; ++i)
        weights[i] = 1.0 / (1.0 + dist[state.positions[i]][request]);

    double total_weight = std::accumulate(weights.begin(), weights.end(), 0.0);
    std::uniform_real_distribution<double> dist_real(0.0, total_weight);
    double r = dist_real(engine);

    double cumsum = 0.0;
    int chosen = state.k - 1;
    for (int i = 0; i < state.k; ++i) {
        cumsum += weights[i];
        if (r <= cumsum) { chosen = i; break; }
    }

    state.move_server(chosen, request, dist);
    return state;
}

// ============================================================
// Optimal Offline (small instances via exhaustive search)
// For k servers on a line with few positions, we can solve
// via dynamic programming over subsets.
// ============================================================

// For small k and small metric space, we use DP.
// State: (request_index, multiset of server positions as sorted tuple)
// We enumerate all possible choices at each step.

long long optimal_offline_server_dp(
    int req_idx,
    std::vector<int> current_positions,
    const std::vector<int>& requests,
    const std::vector<std::vector<int>>& dist,
    std::map<std::vector<int>, long long>& memo)
{
    if (req_idx == static_cast<int>(requests.size())) return 0;

    std::sort(current_positions.begin(), current_positions.end());
    auto key = current_positions;
    // We need to include req_idx in the key; use a composite key via map of maps.
    // Simplification: use a flat map with encoded state.
    // For small instances this suffices.

    int req = requests[req_idx];

    long long best = std::numeric_limits<long long>::max();

    // Option 1: if a server already at req, no cost
    if (std::ranges::find(current_positions, req) != current_positions.end()) {
        best = optimal_offline_server_dp(
            req_idx + 1, current_positions, requests, dist, memo);
        return best;
    }

    // Option 2: move each server to req
    for (int i = 0; i < static_cast<int>(current_positions.size()); ++i) {
        int old = current_positions[i];
        int cost = dist[old][req];
        current_positions[i] = req;
        long long sub = optimal_offline_server_dp(
            req_idx + 1, current_positions, requests, dist, memo);
        best = std::min(best, cost + sub);
        current_positions[i] = old;
    }

    return best;
}

// Wrapper: uses memoization with composite key (req_idx, positions)
long long optimal_offline_server_impl(
    int req_idx,
    std::vector<int> current_positions,
    const std::vector<int>& requests,
    const std::vector<std::vector<int>>& dist,
    std::map<std::pair<int, std::vector<int>>, long long>& memo)
{
    if (req_idx == static_cast<int>(requests.size())) return 0;

    auto state_key = std::make_pair(req_idx, current_positions);
    if (memo.contains(state_key)) return memo[state_key];

    int req = requests[req_idx];
    long long best = std::numeric_limits<long long>::max();

    if (std::ranges::find(current_positions, req) != current_positions.end()) {
        best = optimal_offline_server_impl(
            req_idx + 1, current_positions, requests, dist, memo);
    } else {
        for (int i = 0; i < static_cast<int>(current_positions.size()); ++i) {
            int old = current_positions[i];
            int cost = dist[old][req];
            current_positions[i] = req;
            long long sub = optimal_offline_server_impl(
                req_idx + 1, current_positions, requests, dist, memo);
            best = std::min(best, static_cast<long long>(cost) + sub);
            current_positions[i] = old;
        }
    }

    memo[state_key] = best;
    return best;
}

long long optimal_offline_server(
    int /*k*/,
    const std::vector<int>& initial_positions,
    const std::vector<int>& requests,
    const std::vector<std::vector<int>>& dist)
{
    // Only feasible for small instances
    std::map<std::pair<int, std::vector<int>>, long long> memo;
    return optimal_offline_server_impl(0, initial_positions, requests, dist, memo);
}

// ============================================================
// Utility: build distance matrix for a line metric {0,1,...,n-1}
// ============================================================

std::vector<std::vector<int>> line_metric(int n) {
    std::vector<std::vector<int>> dist(n, std::vector<int>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            dist[i][j] = std::abs(i - j);
    return dist;
}

// ============================================================
// Demonstration
// ============================================================

void demonstrate_k_server() {
    std::println("=== k-Server Problem (Chapter 13) ===\n");

    constexpr int n = 6;  // metric space: line {0,1,2,3,4,5}
    constexpr int k = 2;  // number of servers

    auto dist = line_metric(n);

    // Print distance matrix
    std::println("--- Line Metric (positions 0..{}) ---", n - 1);
    std::print("     ");
    for (int j = 0; j < n; ++j) std::print("{:>4d}", j);
    std::println("");
    for (int i = 0; i < n; ++i) {
        std::print("{:>3d}: ", i);
        for (int j = 0; j < n; ++j) std::print("{:>4d}", dist[i][j]);
        std::println("");
    }
    std::println("");

    // Request sequence
    std::vector<int> requests = {5, 0, 3, 1, 4, 2, 5, 0, 3, 1};
    std::println("--- Request Sequence ---");
    std::print("  ");
    for (int r : requests) std::print("{} ", r);
    std::println("\n");

    // ---- Greedy (closest server) ----
    std::println("--- Greedy (Closest Server) ---");
    {
        ServerState state(k, {0, 1});
        std::println("  Initial: servers at {}", state.snapshot());

        for (int step = 0; auto req : requests) {
            ++step;
            bool hit = state.serves(req);
            state = min_server_move(state, req, dist);
            if (hit) {
                std::println("  Step {:2d}: request {} -> HIT   servers={}",
                             step, req, state.snapshot());
            } else {
                auto& [from, to] = state.movements.back();
                std::println("  Step {:2d}: request {} -> MISS  move {}->{}  servers={}  cost={}",
                             step, req, from, to, state.snapshot(), dist[from][to]);
            }
        }
        std::println("  Total cost: {}\n", state.total_cost);
    }

    // ---- Randomized ----
    std::println("--- Randomized (Inverse-Distance Weighted) ---");
    {
        // Run multiple trials
        constexpr int trials = 10;
        long long total_cost = 0;
        long long best_cost = std::numeric_limits<long long>::max();
        long long worst_cost = 0;

        for (int t = 0; t < trials; ++t) {
            ServerState state(k, {0, 1});
            for (int req : requests)
                state = random_server_move(state, req, dist);
            total_cost += state.total_cost;
            best_cost = std::min(best_cost, state.total_cost);
            worst_cost = std::max(worst_cost, state.total_cost);
        }
        std::println("  {} trials, initial servers at {{0,1}}", trials);
        std::println("  Average cost: {:.1f}", static_cast<double>(total_cost) / trials);
        std::println("  Best cost:  {}", best_cost);
        std::println("  Worst cost: {}", worst_cost);

        // One detailed run
        ServerState state(k, {0, 1});
        std::println("\n  Detailed run:");
        std::println("    Initial: servers at {}", state.snapshot());
        for (int step = 0; auto req : requests) {
            ++step;
            bool hit = state.serves(req);
            state = random_server_move(state, req, dist);
            if (!hit) {
                auto& [from, to] = state.movements.back();
                std::println("    Step {:2d}: request {} -> move {}->{}  servers={}  cost={}",
                             step, req, from, to, state.snapshot(), dist[from][to]);
            } else {
                std::println("    Step {:2d}: request {} -> HIT   servers={}",
                             step, req, state.snapshot());
            }
        }
        std::println("  Total cost: {}\n", state.total_cost);
    }

    // ---- Optimal Offline (small instance) ----
    std::println("--- Optimal Offline (DP, small instance) ---");
    {
        // Use shorter sequence for DP feasibility
        std::vector<int> short_req = {5, 0, 3, 1, 4};
        ServerState greedy_state(k, {0, 1});
        for (int req : short_req)
            greedy_state = min_server_move(greedy_state, req, dist);

        long long opt_cost = optimal_offline_server(k, {0, 1}, short_req, dist);

        std::println("  Requests: {}", short_req);
        std::println("  Initial servers: {{0, 1}}");
        std::println("  Greedy cost: {}", greedy_state.total_cost);
        std::println("  Optimal cost: {}", opt_cost);
        std::println("  Competitive ratio (greedy/opt): {:.2f}",
                     static_cast<double>(greedy_state.total_cost) / opt_cost);
        std::println("");
    }

    // ---- Comparison across different k values ----
    std::println("--- Comparison: k=1,2,3 on line {{0..5}} ---");
    {
        std::vector<int> requests_long = {5, 0, 3, 1, 4, 2, 5, 0, 3, 1, 4, 2};
        std::println("  Requests: {}", requests_long);
        std::println("  {:>4s}  {:>8s}  {:>10s}  {:>8s}",
                     "k", "Greedy", "Random(avg)", "OPT");
        std::println("  {:>4s}  {:>8s}  {:>10s}  {:>8s}",
                     "---", "------", "-----------", "---");

        for (int kval : {1, 2, 3}) {
            // Greedy
            std::vector<int> init_pos;
            for (int i = 0; i < kval; ++i) init_pos.push_back(i);
            ServerState gs(kval, init_pos);
            for (int req : requests_long)
                gs = min_server_move(gs, req, dist);

            // Random (average over trials)
            long long rand_total = 0;
            constexpr int trials = 20;
            for (int t = 0; t < trials; ++t) {
                ServerState rs(kval, init_pos);
                for (int req : requests_long)
                    rs = random_server_move(rs, req, dist);
                rand_total += rs.total_cost;
            }

            // OPT (small enough for DP if kval <= 3 and sequence <= 12)
            long long opt_cost = 0;
            if (kval <= 3 && static_cast<int>(requests_long.size()) <= 12)
                opt_cost = optimal_offline_server(kval, init_pos, requests_long, dist);

            std::println("  {:>4d}  {:>8}  {:>10.1f}  {:>8}",
                         kval, gs.total_cost,
                         static_cast<double>(rand_total) / trials,
                         opt_cost);
        }
        std::println("");
    }

    // ---- Metric space: circle ----
    std::println("--- Alternative Metric: Circle (6 positions) ---");
    {
        // Build circular distance matrix
        auto circle_dist = [](int n) {
            std::vector<std::vector<int>> d(n, std::vector<int>(n));
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j) {
                    int diff = std::abs(i - j);
                    d[i][j] = std::min(diff, n - diff);
                }
            return d;
        };

        auto cdist = circle_dist(6);
        std::vector<int> circ_req = {4, 1, 5, 2, 0, 3};
        std::println("  Circle of 6 positions, requests: {}", circ_req);

        // Greedy
        ServerState gs(2, {0, 1});
        for (int req : circ_req) gs = min_server_move(gs, req, cdist);
        std::println("  Greedy cost: {}", gs.total_cost);

        // Random average
        long long rt = 0;
        for (int t = 0; t < 20; ++t) {
            ServerState rs(2, {0, 1});
            for (int req : circ_req) rs = random_server_move(rs, req, cdist);
            rt += rs.total_cost;
        }
        std::println("  Random avg:  {:.1f}", static_cast<double>(rt) / 20);

        // OPT
        long long opt = optimal_offline_server(2, {0, 1}, circ_req, cdist);
        std::println("  OPT:         {}", opt);
        std::println("");
    }

    std::println("--- Theoretical Results ---");
    std::println("  k-Server Conjecture: no randomized algorithm is k/(k-1)-competitive");
    std::println("  Deterministic lower bound: k (Manasse-McGeoch-Sleator)");
    std::println("  Randomized lower bound: k (Bartal)");
    std::println("  The k-server problem is the canonical online problem\n");
}

} // namespace randalgo
