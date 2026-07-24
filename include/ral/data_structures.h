#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <random>
#include <chrono>

#include "treap.h"
#include "skip_list.h"
#include "hash_table.h"

namespace ral {

// Treap
using Treap = chapter9::Treap;

// SkipList
using SkipList = chapter9::SkipList;

// Hashing
using ChainedHashTable = chapter9::UniversalHashTable;
using PerfectHashTable = chapter9::PerfectHashTable;

} // namespace ral
