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
#include "adversary.h"

// Everything is now directly in namespace ral.
namespace ral {
inline int marking_paging(int cache_size, const std::vector<int>& requests, bool verbose = false) {
    return marking_algorithm(cache_size, requests, verbose);
}
} // namespace ral
