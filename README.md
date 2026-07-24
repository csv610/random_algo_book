# Randomized Algorithms Library (RAL)

A self-contained, header-only C++23 library of randomized algorithms, based on the textbook **"Randomized Algorithms with C++"** by **Chaman Singh Verma**. 

RAL is designed to be highly reusable, modular, and performant, mirroring the structure and ergonomics of the Standard Template Library (STL).

---

## Features

RAL provides implementations for key randomized algorithms across five major domains:

*   **Graph Algorithms (`ral/graph.h`)**:
    *   Karger-Stein Randomized Min-Cut ($O(n^2 \log^3 n)$)
    *   Karger-Klein-Tarjan (KKT) Linear-Time Minimum Spanning Tree
    *   Seidel's All-Pairs Shortest Path (APSP) for unweighted undirected graphs
    *   Luby's Parallel Maximal Independent Set (MIS)
    *   Randomized Maximal & Maximum Matchings
*   **Data Structures (`ral/data_structures.h`)**:
    *   Randomized Treap (Binary Search Tree + Heap)
    *   Skip List (probabilistic alternative to balanced BSTs)
    *   Universal Hash Table (chaining-based)
    *   Perfect Hash Table (Fredman-Komlos-Szemeredi $O(1)$ worst-case lookup)
*   **Algebraic Algorithms (`ral/algebraic.h`)**:
    *   Freivalds' Matrix Multiplication Verification
    *   Schwartz-Zippel Polynomial Identity Testing (PIT)
    *   Rabin-Karp Randomized String Matching
    *   Modular Polynomial Arithmetic, Chinese Remainder Theorem (CRT), and RSA Cryptography
*   **Geometric & LP Algorithms (`ral/geometry.h`)**:
    *   Randomized Incremental Construction (RIC) Convex Hull & Delaunay Triangulation
    *   Seidel's Randomized Linear Programming Solver
    *   Binary Planar Partitions (BSP Trees)
*   **Probability & Online Algorithms (`ral/probability.h`)**:
    *   QuickSort & QuickSelect / LazySelect (expected linear-time selection)
    *   Chernoff & Azuma-Hoeffding Concentration Bounds
    *   Parallel Packet Routing in Hypercubes
    *   Online Paging Algorithms (LRU, FIFO, Marking Paging)
    *   Online K-Server Metric Algorithms & Oblivious/Adaptive Adversaries
*   **Sketching & Streaming Algorithms (`ral/sketching.h`)**:
    *   HyperLogLog (HLL) Cardinality Estimation
    *   Count-Min Sketch Frequency Estimation
    *   MinHash Jaccard Similarity Estimation
*   **Random Projections (`ral/random_projection.h`)**:
    *   Johnson-Lindenstrauss Dimensionality Reduction
*   **Randomized Numerical Linear Algebra (`ral/randomized_linear_algebra.h`)**:
    *   Randomized Range Finder (RandNLA)

---

## Directory Structure

```
RandomAlgo/
  ├── include/              <-- The Header-Only Library (Copy this to your project)
  │    ├── ral.h            <-- Umbrella library header
  │    └── ral/             <-- Module headers (graph.h, data_structures.h, etc.)
  │
  ├── src/                  <-- Chapter benchmarks and verification drivers
  │    ├── chapter1/
  │    ├── chapter2/
  │    └── ...
  │
  ├── examples/             <-- Runnable library usage examples
  ├── Makefile
  └── README.md
```

---

## Installation

Since RAL is header-only, there is no building or linking required. Simply copy the `include/` directory (or the nested `ral.h` and `ral/` folders) into your project's include path.

---

## Quick Start Example

Here is a simple example showing how to use RAL in your own C++ project:

```cpp
#include <iostream>
#include <random>
#include <ral.h>

int main() {
    // 1. Seed a standard C++ random engine
    std::mt19937 rng(1337);

    // 2. Initialize a Randomized Treap
    ral::Treap treap(rng);

    // 3. Perform operations
    treap.insert(10);
    treap.insert(20);
    treap.insert(5);

    std::cout << "Treap Size: " << treap.size() << "\n";
    std::cout << "Searching for 20: " << (treap.search(20) ? "Found" : "Not Found") << "\n";

    return 0;
}
```

### Compiling
Compile using any C++23 compatible compiler (e.g., GCC 13+ or Clang 16+):
```bash
g++ -std=c++23 -Ipath/to/include -o main main.cpp
```

---

## Building and Running Included Examples

The library comes with 6 comprehensive test executables located in the `examples/` directory.

To build and run all examples:
```bash
# Compile the examples
make examples

# Compile and execute
make run-examples
```

---

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
