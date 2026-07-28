// ============================================================
// Example: The K-Server Problem (Chapter 13)
//
// The k-server problem is a fundamental online decision problem.
// We have k servers placed on a metric space. Requests arrive
// one at a time; each request names a point that must be served
// by moving some server there. The goal is to minimize total
// movement cost.
//
// This example compares:
//   1. Greedy (deterministic): always move the closest server.
//   2. Randomized: select a server with probability inversely
//      proportional to its distance to the request.
//   3. Optimal offline (exhaustive DP): solves a small instance
//      exactly, giving a lower bound on cost.
//
// Compile: g++ -std=c++23 -I../include example_k_server.cpp -o example_k_server
// ============================================================

#include "ral/k_server.h"
#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <numeric>

using namespace ral;

int main() {
    std::cout << "=== K-Server Problem: Greedy vs Randomized vs Optimal ===\n\n";

    // --- Setup: 6-point line metric, 2 servers ---
    constexpr int n = 6;
    constexpr int k = 2;

    auto dist = line_metric(n);

    std::cout << "Metric space: line {0,1,2,3,4,5}\n";
    std::cout << "Number of servers: " << k << "\n\n";

    // A sequence of requests the adversary throws at us
    std::vector<int> requests = {5, 0, 3, 1, 4, 2, 5, 0, 3, 1};
    std::cout << "Request sequence: ";
    for (int r : requests) std::cout << r << " ";
    std::cout << "\n\n";

    // --- Strategy 1: Greedy (closest server) ---
    std::cout << "--- Greedy (deterministic, closest server) ---\n";
    {
        ServerState state(k, {0, 1});
        std::cout << "  Initial positions: [";
        for (int i = 0; i < k; ++i) std::cout << (i ? "," : "") << state.positions[i];
        std::cout << "]\n";

        {
            int step = 0;
            for (int req : requests) {
                ++step;
                bool hit = state.serves(req);
                state = min_server_move(state, req, dist);
                if (hit) {
                    std::cout << "  Step " << step << ": request " << req
                              << " -> HIT  (no move)\n";
                } else {
                    auto& [from, to] = state.movements.back();
                    std::cout << "  Step " << step << ": request " << req
                              << " -> move server " << from << "->" << to
                              << "  cost=" << dist[from][to] << "\n";
                }
            }
        }
        std::cout << "  Total cost: " << state.total_cost << "\n\n";
    }

    // --- Strategy 2: Randomized (inverse-distance weighted) ---
    std::cout << "--- Randomized (inverse-distance weighted) ---\n";
    {
        constexpr int trials = 20;
        long long total_cost = 0;
        long long best_cost = std::numeric_limits<long long>::max();
        long long worst_cost = 0;

        for (int t = 0; t < trials; ++t) {
            ServerState state(k, {0, 1});
            for (int req : requests)
                state = random_server_move(state, req, dist);
            total_cost += state.total_cost;
            best_cost  = std::min(best_cost, state.total_cost);
            worst_cost = std::max(worst_cost, state.total_cost);
        }
        std::cout << "  " << trials << " independent trials\n";
        std::cout << "  Average cost: " << static_cast<double>(total_cost) / trials << "\n";
        std::cout << "  Best cost:    " << best_cost << "\n";
        std::cout << "  Worst cost:   " << worst_cost << "\n\n";

        {
            int step = 0;
            ServerState state(k, {0, 1});
            std::cout << "  Detailed run:\n";
            for (int req : requests) {
                ++step;
                bool hit = state.serves(req);
                state = random_server_move(state, req, dist);
                if (hit) {
                    std::cout << "    Step " << step << ": request " << req
                              << " -> HIT\n";
                } else {
                    auto& [from, to] = state.movements.back();
                    std::cout << "    Step " << step << ": request " << req
                              << " -> move " << from << "->" << to
                              << "  cost=" << dist[from][to] << "\n";
                }
            }
            std::cout << "  Total cost: " << state.total_cost << "\n\n";
        }
    }

    // --- Strategy 3: Optimal offline (exhaustive DP) ---
    std::cout << "--- Optimal Offline (DP, exhaustive search) ---\n";
    {
        // Use a shorter request sequence so the DP is tractable
        std::vector<int> short_req = {5, 0, 3, 1, 4};

        // Greedy cost for comparison
        ServerState greedy_state(k, {0, 1});
        for (int req : short_req)
            greedy_state = min_server_move(greedy_state, req, dist);

        long long opt_cost = optimal_offline_server(k, {0, 1}, short_req, dist);

        std::cout << "  Requests: ";
        for (int r : short_req) std::cout << r << " ";
        std::cout << "\n";
        std::cout << "  Greedy cost:  " << greedy_state.total_cost << "\n";
        std::cout << "  Optimal cost: " << opt_cost << "\n";
        std::cout << "  Competitive ratio (greedy/opt): "
                  << static_cast<double>(greedy_state.total_cost) / opt_cost << "\n\n";
    }

    // --- Comparison across different k values ---
    std::cout << "--- Comparison: k=1,2,3 on line {0..5} ---\n";
    {
        std::vector<int> long_req = {5, 0, 3, 1, 4, 2, 5, 0, 3, 1, 4, 2};
        std::cout << "  Requests: ";
        for (int r : long_req) std::cout << r << " ";
        std::cout << "\n";
        std::cout << "     k   Greedy  Random(avg)       OPT\n";
        std::cout << "   ---   ------  -----------       ---\n";

        for (int kval : {1, 2, 3}) {
            std::vector<int> init_pos;
            for (int i = 0; i < kval; ++i) init_pos.push_back(i);

            // Greedy
            ServerState gs(kval, init_pos);
            for (int req : long_req)
                gs = min_server_move(gs, req, dist);

            // Randomized (average)
            long long rand_total = 0;
            constexpr int trials = 20;
            for (int t = 0; t < trials; ++t) {
                ServerState rs(kval, init_pos);
                for (int req : long_req)
                    rs = random_server_move(rs, req, dist);
                rand_total += rs.total_cost;
            }

            // Optimal (feasible for k<=3 and sequence length <= 12)
            long long opt_cost = 0;
            if (kval <= 3 && static_cast<int>(long_req.size()) <= 12)
                opt_cost = optimal_offline_server(kval, init_pos, long_req, dist);

            printf("   %d   %6lld  %10.1f  %8lld\n",
                   kval, gs.total_cost,
                   static_cast<double>(rand_total) / trials,
                   opt_cost);
        }
        std::cout << "\n";
    }

    std::cout << "--- Theoretical Results ---\n";
    std::cout << "  k-Server Conjecture: no randomized algorithm is k/(k-1)-competitive\n";
    std::cout << "  Deterministic lower bound: k  (Manasse-McGeoch-Sleator)\n";
    std::cout << "  Randomized lower bound: k    (Bartal)\n";

    return 0;
}
