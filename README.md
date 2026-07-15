# Randomized Algorithms

![Front Page](frontpage.png)

## About

This book examines randomized algorithms: algorithms that make random choices during execution. It covers theoretical foundations, probabilistic analysis, and applications in computer science.

Randomized algorithms use randomness to solve problems. They can be simpler or faster than deterministic alternatives for certain problems. This work presents their analysis with supporting implementations.

## Chapter 1: Introduction

### Topics

#### 1.1 Min-Cut Algorithm (Karger)
A randomized algorithm for finding minimum cuts in graphs. Uses edge contraction with random selection. Analysis includes success probability and failure bounds.

#### 1.2 Las Vegas and Monte Carlo Algorithms
Two classes of randomized algorithms. Las Vegas algorithms always return correct results but have variable runtime. Monte Carlo algorithms have fixed runtime but may return incorrect results with bounded probability.

#### 1.3 Binary Planar Partitions
Randomized construction of binary space partitions. Application: hidden line elimination in computer graphics. Analysis uses linearity of expectation.

#### 1.4 Probabilistic Recurrence Relations
Analysis of randomized algorithms through recurrence relations. Studies expected running time and recursion depth. Uses geometric distribution.

#### 1.5 Complexity Classes
Randomized complexity classes: RP, co-RP, ZPP, BPP. Relationships to P and NP.

## Source Code

```
src/chapter1/
├── random_utils.h           # Random number generation
├── min_cut.h                # Karger's Min-Cut Algorithm
├── las_vegas_monte_carlo.h  # Las Vegas and Monte Carlo examples
├── binary_planar_partition.h # Binary Planar Partitions
├── probabilistic_recurrence.h # Selection and recurrence analysis
└── main.cpp                 # Demonstrations and validation
```

## Building

### Make
```bash
make chapter1      # Build
make run-chapter1  # Build and run
```

### CMake
```bash
mkdir build && cd build
cmake ..
make
./chapter1
```

### Direct Compilation
```bash
g++ -std=c++17 -O2 -o chapter1 src/chapter1/main.cpp -Isrc/chapter1
./chapter1
```

## Key Concepts

1. **Randomization** — Use of random choices in algorithms
2. **Concentration Bounds** — Chernoff, Hoeffding inequalities
3. **Linearity of Expectation** — Expectation of sum equals sum of expectations
4. **Geometric Distribution** — Number of trials until first success
5. **Probabilistic Method** — Proving existence through random construction
