# Split Multi-Application Examples into Single-Application Examples

> **For Hermes:** Use subagent-driven-development skill to implement this plan task-by-task.

**Goal:** Split 6 multi-application example files into 29 single-application examples so each example demonstrates one algorithm clearly, making it easier for students to learn incrementally.

**Architecture:** Each new example file will be self-contained, focused on one algorithm/application, with clear output and pedagogical structure.

**Tech Stack:** C++23, CMake, RAL library (include/ral/)

---

### Task 1: Split probability_example.cpp (6 applications)

**Current:** `examples/probability_example.cpp` (5443 bytes, 122 lines)

**Applications to extract:**
1. Las Vegas Sorting (Randomized Quicksort)
2. LazySelect Selection Algorithm
3. Monte Carlo Pi Estimation
4. Chernoff Bounds / Concentration Inequalities
5. Online Paging Algorithms (LRU, FIFO, Random, Marking)
6. Online k-Server Problem

**Files:**
- Create: `examples/las_vegas_quicksort.cpp`
- Create: `examples/lazy_select.cpp`
- Create: `examples/monte_carlo_pi.cpp`
- Create: `examples/chernoff_bounds.cpp`
- Create: `examples/online_paging.cpp`
- Create: `examples/k_server.cpp`
- Delete: `examples/probability_example.cpp`

**Step 1: Write failing test (verify build)**

Run: `cmake -B build_cmake && make probability_example`
Expected: FAIL - file deleted

**Step 2: Create each example file with minimal implementation**

Each file includes:
- Header with algorithm name
- Minimal working example
- Clear output showing the concept

**Step 3: Add to CMakeLists.txt**

Add 6 new targets to EXAMPLES list

**Step 4: Build and verify**

Run: `make las_vegas_quicksort lazy_select monte_carlo_pi chernoff_bounds online_paging k_server`
Expected: All 6 build and run

**Step 5: Commit**

```bash
git add examples/ CMakeLists.txt
git commit -m "feat: split probability_example into 6 single-application examples"
```

---

### Task 2: Split geometry_example.cpp (4 applications)

**Current:** `examples/geometry_example.cpp` (3805 bytes, 99 lines)

**Applications to extract:**
1. Convex Hull (RIC + Jarvis)
2. Delaunay Triangulation (Bowyer-Watson)
3. Seidel's Linear Programming
4. Binary Planar Partitions (BSP)

**Files:**
- Create: `examples/convex_hull.cpp`
- Create: `examples/delaunay_triangulation.cpp`
- Create: `examples/seidel_lp.cpp`
- Create: `examples/bsp_partition.cpp`
- Delete: `examples/geometry_example.cpp`

**Step 1-5:** Same pattern as Task 1

---

### Task 3: Split modern_algorithms_example.cpp (5 applications)

**Current:** `examples/modern_algorithms_example.cpp` (6344 bytes, 142 lines)

**Applications to extract:**
1. MinHash Jaccard Similarity
2. Count-Min Sketch Frequency Estimation
3. HyperLogLog Cardinality Estimation
4. Johnson-Lindenstrauss Random Projection
5. Randomized Range Finder (RandNLA)

**Files:**
- Create: `examples/minhash.cpp`
- Create: `examples/count_min_sketch.cpp`
- Create: `examples/hyperloglog.cpp`
- Create: `examples/jl_projection.cpp`
- Create: `examples/randnla.cpp`
- Delete: `examples/modern_algorithms_example.cpp`

**Step 1-5:** Same pattern

---

### Task 4: Split algebraic_example.cpp (5 applications)

**Current:** `examples/algebraic_example.cpp` (5042 bytes, 105 lines)

**Applications to extract:**
1. Freivalds' Matrix Verification
2. Schwartz-Zippel Polynomial Identity Testing
3. Rabin-Karp String Matching
4. Modular Polynomial Arithmetic
5. Number Theory & RSA Cryptography

**Files:**
- Create: `examples/freivalds_verification.cpp`
- Create: `examples/schwartz_zippel_pit.cpp`
- Create: `examples/rabin_karp.cpp`
- Create: `examples/modular_polynomial.cpp`
- Create: `examples/rsa_crypto.cpp`
- Delete: `examples/algebraic_example.cpp`

**Step 1-5:** Same pattern

---

### Task 5: Split data_structures_example.cpp (4 applications)

**Current:** `examples/data_structures_example.cpp` (4435 bytes, 102 lines)

**Applications to extract:**
1. Random Treaps
2. Skip Lists
3. Universal Hash Tables (Chained)
4. Perfect Hash Tables

**Files:**
- Create: `examples/treap.cpp`
- Create: `examples/skip_list.cpp`
- Create: `examples/universal_hash.cpp`
- Create: `examples/perfect_hash.cpp`
- Delete: `examples/data_structures_example.cpp`

**Step 1-5:** Same pattern

---

### Task 6: Split graph_example.cpp (5 applications)

**Current:** `examples/graph_example.cpp` (4235 bytes, 111 lines)

**Applications to extract:**
1. Karger-Stein Min-Cut
2. Karger-Klein-Tarjan MST
3. Seidel's APSP (Unweighted)
4. Luby's Parallel MIS
5. Randomized Matchings

**Files:**
- Create: `examples/karger_stein_mincut.cpp`
- Create: `examples/kkt_mst.cpp`
- Create: `examples/seidel_apsp.cpp`
- Create: `examples/luby_mis.cpp`
- Create: `examples/randomized_matching.cpp`
- Delete: `examples/graph_example.cpp`

**Step 1-5:** Same pattern

---

### Task 7: Update CMakeLists.txt

**Files:**
- Modify: `CMakeLists.txt` (lines 42-55)

Replace EXAMPLES list with all 29 new examples + 3 existing singles:
```
karger_min_cut
karger_stein_min_cut
las_vegas_monte_carlo
binary_planar_partition
randomized_find
las_vegas_quicksort
lazy_select
monte_carlo_pi
chernoff_bounds
online_paging
k_server
convex_hull
delaunay_triangulation
seidel_lp
bsp_partition
minhash
count_min_sketch
hyperloglog
jl_projection
randnla
freivalds_verification
schwartz_zippel_pit
rabin_karp
modular_polynomial
rsa_crypto
treap
skip_list
universal_hash
perfect_hash
karger_stein_mincut
kkt_mst
seidel_apsp
luby_mis
randomized_matching
modern_algorithms_example
benchmarks
```

**Step 1:** Edit CMakeLists.txt with complete list
**Step 2:** `cmake -B build_cmake && make`
**Step 3:** Verify all 32 targets build
**Step 4:** Commit

---

### Task 8: Verify all examples run and produce output

**Step 1:** Run each example to verify output
```bash
cd build_cmake
for exe in karger_min_cut karger_stein_min_cut las_vegas_monte_carlo \
  binary_planar_partition randomized_find las_vegas_quicksort lazy_select \
  monte_carlo_pi chernoff_bounds online_paging k_server convex_hull \
  delaunay_triangulation seidel_lp bsp_partition minhash count_min_sketch \
  hyperloglog jl_projection randnla freivalds_verification schwartz_zippel_pit \
  rabin_karp modular_polynomial rsa_crypto treap skip_list universal_hash \
  perfect_hash karger_stein_mincut kkt_mst seidel_apsp luby_mis \
  randomized_matching modern_algorithms_example benchmarks; do
  echo "=== $exe ===" && ./$exe 2>&1 | head -20
done
```

**Step 2:** Confirm all produce meaningful output
**Step 3:** Final commit

---

## Validation

**Tests:** Each example should build independently and produce console output demonstrating its algorithm.

**Risks:**
- Some examples may need ral:: namespace adjustments
- Dependencies between examples (e.g., both min-cut examples use karger_stein)
- Build time increases with 32 targets

**Open Questions:**
- Keep `modern_algorithms_example.cpp` as-is or also split?
- Keep `benchmarks.cpp` as-is or split by category?
- Rename `karger_stein_min_cut.cpp` vs `karger_stein_mincut.cpp` conflict?

---

## Summary

**Total new files:** 29 created, 6 deleted = 23 net new example files
**Total examples:** 32 (was 11)
**Approach:** TDD per example, commit per task, verify build at each step