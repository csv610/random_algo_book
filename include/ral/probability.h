#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <climits>
#include <atomic>
#include <thread>
#include <latch>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <functional>

#include "las_vegas_monte_carlo.h"
#include "probabilistic_recurrence.h"
#include "lazy_select.h"
#include "coupon_collector.h"
#include "two_point.h"
#include "chernoff.h"
#include "martingales.h"
#include "routing.h"
#include "pram_simulation.h"
#include "paging.h"
#include "k_server.h"
#include "adversary.h"

namespace ral {

// Las Vegas & Monte Carlo Examples
using chapter1::randomized_quicksort;
using chapter1::randomized_min_cut_monte_carlo;
using chapter1::estimate_pi_monte_carlo;

// Selection
using chapter1::randomized_find; // QuickSelect
using SelectResult = chapter4::SelectResult;
using chapter4::lazy_select;

// Coupon Collector
using chapter4::simulate_coupon_collector;

// Two-Point Sampling / Pairwise Independence
using chapter4::RP_algorithm;
using chapter4::naive_two_point;
using chapter4::amplified_two_point;

// Tail Bounds (Chernoff, Martingales)
using chapter5::chernoff_upper;
using chapter5::chernoff_simplified;
using chapter5::mgf_bound;
using chapter5::azuma_hoeffding_bound;

// Routing in Parallel Computes
using chapter5::simulate_routing;
using chapter5::simulate_routing_rounds;

// PRAM Simulation
using PRAM = randalgo::PRAM;

// Online Paging Algorithms
using randalgo::random_paging;
inline int marking_paging(int cache_size, const std::vector<int>& requests, bool verbose = false) {
    return randalgo::marking_algorithm(cache_size, requests, verbose);
}
using randalgo::deterministic_lru;
using randalgo::deterministic_fifo;

// Online K-Server Algorithms
using ServerState = randalgo::ServerState;
using randalgo::min_server_move;
using randalgo::random_server_move;
using randalgo::optimal_offline_server;

// Online Adversaries
using ObliviousAdversary = randalgo::ObliviousAdversary;
using AdaptiveOnlineAdversary = randalgo::AdaptiveOnlineAdversary;

} // namespace ral
