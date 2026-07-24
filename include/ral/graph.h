#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <climits>
#include <cmath>
#include <iomanip>
#include <random>
#include <chrono>
#include <queue>
#include <ranges>
#include <span>
#include <bit>

#include "min_cut_chapter1.h"
#include "min_cut.h"
#include "mst.h"
#include "apsp.h"
#include "mis.h"
#include "matchings.h"

namespace ral {

// Graph representations
using Graph = randalgo::Graph;
using Multigraph = randalgo::Multigraph;
using WeightedGraph = randalgo::WeightedGraph;
using KargerGraph = chapter1::Graph;

// Types
using Edge = randalgo::Edge;
using Matrix = randalgo::Matrix;
using BoolMatrix = randalgo::BoolMatrix;
using IndependentSet = randalgo::IndependentSet;
using Matching = randalgo::Matching;

// Algorithms
using chapter1::karger_min_cut;

using randalgo::karger_min_cut_size;
using randalgo::karger_repeated;
using randalgo::karger_stein;

using randalgo::kkt_mst;

using randalgo::seidel_apsp_unweighted;
using randalgo::floyd_warshall;
using randalgo::apsp_repeated_squaring;

using randalgo::mis_luby_parallel;

using randalgo::random_maximal_matching;
using randalgo::blossom_matching;
using randalgo::approximate_max_matching;

} // namespace ral
