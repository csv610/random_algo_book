#include "ral.h"
#include <iostream>
#include <vector>
#include <random>

int main() {
    std::cout << "========================================================\n";
    std::cout << "        RAL Probability & Selection Examples\n";
    std::cout << "========================================================\n\n";

    std::mt19937 rng(42);

    // ----------------------------------------------------------------
    // 1. Las Vegas Sorting & Selection
    // ----------------------------------------------------------------
    std::cout << "--- 1. Las Vegas Sorting & Selection ---\n";
    std::vector<int> arr = {9, 7, 5, 11, 12, 2, 14, 3, 10, 6};
    std::cout << "  Input array: ";
    for (int x : arr) std::cout << x << " ";
    std::cout << "\n";

    // QuickSelect (finding 4th smallest element)
    std::vector<int> arr_select = arr;
    int k = 4;
    int k_val = ral::randomized_find(arr_select, k);
    std::cout << "  QuickSelect found " << k << "-th smallest: " << k_val << "\n";

    // Quicksort
    std::vector<int> arr_sort = arr;
    ral::randomized_quicksort(arr_sort, 0, arr_sort.size() - 1);
    std::cout << "  Randomized Quicksort output: ";
    for (int x : arr_sort) std::cout << x << " ";
    std::cout << "\n\n";

    // ----------------------------------------------------------------
    // 2. LazySelect
    // ----------------------------------------------------------------
    std::cout << "--- 2. LazySelect (Using Samples) ---\n";
    std::vector<int> arr_lazy = arr;
    auto select_res = ral::lazy_select(arr_lazy, k, rng);
    std::cout << "  LazySelect found " << k << "-th smallest: " << select_res.value
              << " in " << select_res.comparisons << " comparisons (succeeded in pass "
              << select_res.pass << ")\n\n";

    // ----------------------------------------------------------------
    // 3. Monte Carlo Pi Estimation
    // ----------------------------------------------------------------
    std::cout << "--- 3. Monte Carlo Pi Estimation ---\n";
    int pi_samples = 100000;
    double pi_est = ral::estimate_pi_monte_carlo(pi_samples);
    std::cout << "  Pi estimation with " << pi_samples << " samples: " << pi_est << "\n\n";

    // ----------------------------------------------------------------
    // 4. Tail Bounds & Concentration Inequalities
    // ----------------------------------------------------------------
    std::cout << "--- 4. Tail Bounds & Concentration ---\n";
    double mu = 10.0;
    double delta = 0.5;
    double chernoff_up = ral::chernoff_upper(mu, delta, true);
    double chernoff_simp = ral::chernoff_simplified(mu, delta, true);
    std::cout << "  Chernoff upper tail bound (mu=10, delta=0.5): " << chernoff_up << "\n";
    std::cout << "  Chernoff simplified bound: " << chernoff_simp << "\n";

    // Azuma-Hoeffding (Martingales)
    int n_steps = 100;
    double c_step_max = 1.0;
    double dev_t = 15.0;
    double azuma_bound = ral::azuma_hoeffding_bound(n_steps, c_step_max, dev_t);
    std::cout << "  Azuma-Hoeffding bound (n=100, max_diff=1, t=15): " << azuma_bound << "\n\n";

    // ----------------------------------------------------------------
    // 5. Online Paging Algorithms
    // ----------------------------------------------------------------
    std::cout << "--- 5. Online Paging (LRU, FIFO, Rand, Marking) ---\n";
    int cache_size = 3;
    std::vector<int> requests = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};

    int lru_misses = ral::deterministic_lru(cache_size, requests);
    int fifo_misses = ral::deterministic_fifo(cache_size, requests);
    int rand_misses = ral::random_paging(cache_size, requests);
    int marking_misses = ral::marking_paging(cache_size, requests);

    std::cout << "  Cache Size: " << cache_size << ", Requests size: " << requests.size() << "\n";
    std::cout << "  LRU misses: " << lru_misses << "\n";
    std::cout << "  FIFO misses: " << fifo_misses << "\n";
    std::cout << "  Random Paging misses: " << rand_misses << "\n";
    std::cout << "  Marking Paging misses: " << marking_misses << "\n\n";

    // ----------------------------------------------------------------
    // 6. Online K-Server
    // ----------------------------------------------------------------
    std::cout << "--- 6. Online K-Server ---\n";
    // Distance matrix for 3 points
    std::vector<std::vector<int>> dist = {
        {0, 2, 5},
        {2, 0, 4},
        {5, 4, 0}
    };
    std::vector<int> server_reqs = {2, 0, 1, 2, 0, 1};
    ral::ServerState state(2, {0, 1}); // 2 servers initially at {0, 1}

    std::cout << "  Greedy move cost for sequence: ";
    int total_cost_greedy = 0;
    auto greedy_state = state;
    for (int req : server_reqs) {
        auto next_state = ral::min_server_move(greedy_state, req, dist);
        // Find which server moved and add distance
        for (size_t i = 0; i < greedy_state.positions.size(); ++i) {
            if (greedy_state.positions[i] != next_state.positions[i]) {
                total_cost_greedy += dist[greedy_state.positions[i]][next_state.positions[i]];
            }
        }
        greedy_state = next_state;
    }
    std::cout << total_cost_greedy << "\n";

    long long opt_cost = ral::optimal_offline_server(2, {0, 1}, server_reqs, dist);
    std::cout << "  Optimal offline server cost: " << opt_cost << "\n\n";

    std::cout << "========================================================\n\n";
    return 0;
}
