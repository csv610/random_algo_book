# Randomized Algorithms - C++ Implementation

![Front Page](frontpage.png)

C++ implementation of examples and algorithms from "Randomized Algorithms" by Rajeev Motwani and Prabhakar Raghavan, with working code instead of pseudocode.

## Chapter 1: Introduction

### Files Structure

```
src/chapter1/
├── random_utils.h           # Random number generation utilities
├── min_cut.h                # Karger's Min-Cut Algorithm
├── las_vegas_monte_carlo.h  # Las Vegas vs Monte Carlo examples
├── binary_planar_partition.h # Binary Planar Partitions
├── probabilistic_recurrence.h # Find algorithm and analysis
└── main.cpp                 # Main driver with all demonstrations
```

### What's Implemented

#### 1.1 Min-Cut Algorithm (Karger's Algorithm)
- **File**: `min_cut.h`
- **Algorithm**: Randomized edge contraction to find minimum cut
- **Key Insight**: Probability of success >= 2/n² per trial
- **Demonstration**: Shows single run vs repeated runs with failure probability analysis

#### 1.2 Las Vegas and Monte Carlo Algorithms
- **File**: `las_vegas_monte_carlo.h`
- **Las Vegas Example**: Randomized QuickSort (always correct, random running time)
- **Monte Carlo Examples**: 
  - Pi estimation (two-sided error)
  - Randomized min-cut (one-sided error)
- **Conversion**: Monte Carlo to Las Vegas using verification (Exercise 1.3)

#### 1.3 Binary Planar Partitions
- **File**: `binary_planar_partition.h`
- **Application**: Hidden line elimination in computer graphics
- **Algorithm**: RandAuto - randomized auto-partition
- **Key Insight**: Expected size O(n log n) using linearity of expectation
- **Example**: Sailor problem (Example 1.1) demonstrating indicator variables

#### 1.4 Probabilistic Recurrence
- **File**: `probabilistic_recurrence.h`
- **Algorithm**: Randomized Find (select kth smallest)
- **Analysis**: Expected O(n) time, O(log n) recursion depth
- **Theorem 1.3**: Bounds expected steps using integral formula
- **Connection**: Geometric distribution (expected attempts = 1/p)

#### 1.5 Computation Model and Complexity Classes
- **Coverage**: Theoretical section (no code, but discussed in output)
- **Classes**: P, NP, RP, co-RP, ZPP, BPP, PP
- **Relationships**: P ⊆ RP ⊆ NP ⊆ PSPACE ⊆ EXP ⊆ NEXP

### Building and Running

#### Using Make
```bash
make chapter1      # Build
make run-chapter1  # Build and run
```

#### Using CMake
```bash
mkdir build && cd build
cmake ..
make
./chapter1
```

#### Direct Compilation
```bash
g++ -std=c++17 -O2 -o chapter1 src/chapter1/main.cpp -Isrc/chapter1
./chapter1
```

### Sample Output

The program demonstrates all algorithms with empirical validation:

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

### Key Concepts Demonstrated

1. **Randomization can simplify algorithms** (Min-cut vs network flow)
2. **Error probability reduction** through independent repetition
3. **Linearity of expectation** (no independence required)
4. **Geometric distribution** in algorithm analysis
5. **Probabilistic method** for proving existence of solutions

### References

- Motwani, R. and Raghavan, P. (1995). *Randomized Algorithms*. Cambridge University Press.
- Chapter 1: Introduction (pages 3-27)