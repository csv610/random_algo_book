# Randomized Algorithms

![Front Page](frontpage.png)

C++ implementation of randomized algorithms and techniques presented in "Randomized Algorithms". Each algorithm includes complete, compilable code alongside theoretical analysis.

## Chapter 1: Introduction

### Source Structure

```
src/chapter1/
├── random_utils.h           # Random number generation utilities
├── min_cut.h                # Karger's Min-Cut Algorithm
├── las_vegas_monte_carlo.h  # Las Vegas and Monte Carlo paradigms
├── binary_planar_partition.h # Binary Planar Partitions
├── probabilistic_recurrence.h # Find algorithm and analysis
└── main.cpp                 # Main driver with all demonstrations
```

### Implemented Algorithms

#### 1.1 Min-Cut Algorithm (Karger)
- **Source**: `min_cut.h`
- **Method**: Randomized edge contraction
- **Complexity**: Probability of success ≥ 2/n² per trial
- **Validation**: Single trial versus repeated trials with failure probability analysis

#### 1.2 Las Vegas and Monte Carlo Algorithms
- **Source**: `las_vegas_monte_carlo.h`
- **Las Vegas**: Randomized QuickSort (correct result, stochastic runtime)
- **Monte Carlo**: Pi estimation (two-sided error), randomized min-cut (one-sided error)
- **Conversion**: Monte Carlo to Las Vegas via verification

#### 1.3 Binary Planar Partitions
- **Source**: `binary_planar_partition.h`
- **Application**: Hidden line elimination in computer graphics
- **Method**: RandAuto randomized auto-partition
- **Complexity**: Expected size O(n log n) via linearity of expectation
- **Example**: Sailor problem demonstrating indicator variable analysis

#### 1.4 Probabilistic Recurrence
- **Source**: `probabilistic_recurrence.h`
- **Algorithm**: Randomized selection (kth smallest element)
- **Complexity**: Expected O(n) time, O(log n) recursion depth
- **Analysis**: Theorem 1.3 bounds expected steps via integral formula
- **Distribution**: Geometric distribution (expected attempts = 1/p)

#### 1.5 Computational Complexity Classes
- **Content**: Theoretical discussion (no implementation)
- **Classes**: P, NP, RP, co-RP, ZPP, BPP, PP
- **Hierarchy**: P ⊆ RP ⊆ NP ⊆ PSPACE ⊆ EXP ⊆ NEXP

## Build Instructions

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

## Output

The program validates each algorithm empirically:

```
Section 1.1: Min-Cut Algorithm
- Karger's algorithm on 5-vertex graph
- Single run vs n²/2 trials
- Theoretical vs empirical failure probability

Section 1.2: Las Vegas vs Monte Carlo
- QuickSort: Always correct, O(n log n) expected
- Pi estimation: Error decreases with samples
- Monte Carlo to Las Vegas conversion

Section 1.3: Binary Planar Partitions
- RandAuto on 3 line segments
- Sailor problem: Expected 1 sailor in own cabin

Section 1.4: Probabilistic Recurrence
- Find algorithm: ~12 recursive calls for n=1000
- Geometric distribution validation
```

## Key Concepts

1. **Randomization as algorithmic tool** — Simplifies problems (min-cut vs. network flow)
2. **Error probability reduction** — Independent repetition reduces failure rate
3. **Linearity of expectation** — Applies without independence requirement
4. **Geometric distribution** — Fundamental to randomized algorithm analysis
5. **Probabilistic method** — Proves existence of solutions constructively
