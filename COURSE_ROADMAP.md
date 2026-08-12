# Randomized Algorithms: Student-Centered Course Roadmap

This roadmap expands the book to cover the core expectations represented by
MIT 6.856J, Stanford CS265/CME309, and Berkeley CS174. Existing chapters and
topics are retained. New material is added through supplements, cross-links,
exercises, and advanced modules.

## Student expectations

By the end of the core path, a student should be able to:

- choose between Las Vegas, Monte Carlo, and amplification strategies;
- calculate expectation, variance, tail bounds, and failure probabilities;
- recognize when independence is unavailable and use pairwise independence,
  martingales, coupling, or the second-moment method;
- design and prove randomized algorithms for graph, algebraic, geometric,
  sampling, and data-stream problems;
- state assumptions, correctness guarantees, runtime, space, and error bounds;
- implement representative algorithms and test their behavior experimentally;
- solve proof-oriented problems at upper-division and graduate level.

## Core path

The recommended student sequence is:

1. Randomized computation, probability review, and basic examples.
2. Expectation, variance, sampling, occupancy, and median finding.
3. Markov, Chebyshev, Chernoff, Hoeffding, and moment-generating functions.
4. Poissonization, power-of-two choices, random graphs, and second moments.
5. The probabilistic method, conditional expectations, and the Local Lemma.
6. Pairwise independence, universal hashing, streaming, and sketches.
7. Markov chains, coupling, mixing, martingales, and stopping times.
8. Randomized graph, algebraic, geometric, and counting algorithms.
9. Derandomization, parallel algorithms, online algorithms, and applications.

## Staged expansion

### Stage 1 — map and foundations

- Freeze the existing topic inventory.
- Add this student-facing roadmap.
- Mark every topic as existing core, existing advanced, or planned addition.
- Add prerequisites, learning outcomes, notation conventions, and a reading map.

### Stage 2 — probability toolkit

Add Poisson approximation, Poissonization, power-of-two choices, stochastic
domination, negative dependence, the second-moment method, stopping times,
optional stopping, Doob martingales, and bounded-difference tools.

### Stage 3 — classical randomized algorithms

Add randomized Quicksort, randomized FIND, Yao's principle, randomized binary
search, error amplification, pseudorandomness, and foundational lower bounds.

### Stage 4 — hashing and streaming

Add cuckoo hashing, consistent hashing, Bloom filters, Count-Min Sketch,
Count-Sketch, heavy hitters, frequency moments, distinct-element estimation,
network coding, and polling algorithms.

### Stage 5 — random graphs and derandomization

Add threshold phenomena, Hamilton cycles, random graph coloring, Ramsey bounds,
expander codes, constructive LLL proofs, entropic LLL, symmetry breaking,
Ethernet backoff, bit fixing, and limited-independence derandomization.

### Stage 6 — Markov chains and sampling

Add conductance, spectral gap, strong stationary times, card shuffling,
hypercube walks, Glauber dynamics, Metropolis sampling, randomized 2-SAT,
graph-coloring chains, rapid mixing, and sampling/counting reductions.

### Stage 7 — graph, geometry, and counting completion

Add recursive and sampling-based min-cut methods, network reliability, random
spanning trees, Kirchhoff's theorem, metric embeddings, Bourgain embeddings,
point location, rare-event estimation, and counting-versus-generation.

### Stage 8 — learning and verification layer

For each module, add worked examples, proof exercises, implementation tasks,
research extensions, references, source-course tags, and compilation/tests.

## Non-negotiable preservation rules

1. No existing topic is deleted.
2. Existing advanced material remains available through an advanced path.
3. New material is labeled as supplement, bridge, or advanced module.
4. Every theorem states assumptions, guarantee, error probability, and runtime.
5. Every core module includes student exercises and at least one implementation.
6. Each completed stage is documented, verified, committed, and uploaded.

## Reference course pages

- MIT 6.856J: <https://courses.csail.mit.edu/6.856/>
- Stanford CS265/CME309: <https://web.stanford.edu/class/cs265/>
- Berkeley CS174: <https://people.eecs.berkeley.edu/~sinclair/cs174/s25.html>
