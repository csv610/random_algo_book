# Randomized Algorithms

![Front Page](frontpage.png)

A comprehensive treatment of randomized algorithms, covering foundational theory, probabilistic analysis, and practical applications across computational domains.

## About

Randomized algorithms employ randomness as a computational resource, achieving performance or simplicity unattainable by deterministic methods. This work presents the theory and analysis of randomized algorithms, with supporting implementations for empirical validation.

## Chapter 1: Introduction

### Topics Covered

#### 1.1 Min-Cut Algorithm (Karger)
Randomized edge contraction for computing minimum cuts in graphs. Analysis of success probability and failure bounds through repeated independent trials.

#### 1.2 Las Vegas and Monte Carlo Paradigms
Classification of randomized algorithms by correctness and runtime guarantees. Las Vegas algorithms produce correct results with stochastic runtime; Monte Carlo algorithms provide bounded error with deterministic runtime. Includes conversion techniques between paradigms.

#### 1.3 Binary Planar Partitions
Randomized construction of binary space partitions for hidden line elimination. Application of linearity of expectation to bound expected partition size.

#### 1.4 Probabilistic Recurrence Relations
Analysis of randomized selection algorithms through recurrence relations. Geometric distribution and its role in expected-case complexity.

#### 1.5 Computational Complexity Classes
Theoretical framework for randomized complexity: P, NP, RP, co-RP, ZPP, BPP, PP. Relationships between deterministic, randomized, and nondeterministic computation.

## Repository Structure

```
src/chapter1/
├── random_utils.h           # Random number generation
├── min_cut.h                # Karger's Min-Cut Algorithm
├── las_vegas_monte_carlo.h  # Las Vegas and Monte Carlo examples
├── binary_planar_partition.h # Binary Planar Partitions
├── probabilistic_recurrence.h # Selection and recurrence analysis
└── main.cpp                 # Demonstrations and validation
```

## Building and Running

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

1. **Randomization** — Algorithmic design using probabilistic choice
2. **Concentration Bounds** — Chernoff, Hoeffding, and moment inequalities
3. **Linearity of Expectation** — Linearity without independence requirement
4. **Geometric Distribution** — Waiting time analysis in repeated trials
5. **Probabilistic Method** — Existence proofs via randomized construction
