#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <cassert>
#include <set>

#include "convex_hull.h"
#include "delaunay.h"
#include "linear_programming.h"
#include "binary_planar_partition.h"

namespace ral {

// Basic structures
using Point2D = chapter10::Point2D;
using Triangle = chapter10::Triangle;
using Segment = chapter1::Segment;
using PartitionNode = chapter1::PartitionNode;

// Convex Hull
using chapter10::cross_product;
using chapter10::dist2;
using chapter10::gift_wrap_hull;
using chapter10::convex_hull;

// Delaunay Triangulation
using chapter10::in_circumcircle;
using chapter10::delaunay_triangulation;

// Linear Programming (Seidel)
using LPResult = chapter10::LPResult;
using chapter10::satisfies;
using chapter10::solve_equality_system;
using chapter10::randomized_lp;
using chapter10::solve_lp;

// Binary Planar Partitions
using chapter1::segments_intersect;
using chapter1::rand_auto_partition;
using chapter1::partition_size;

} // namespace ral
