#pragma once
#include <vector>
#include <random>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <cassert>
#include "compat_print.h"

namespace ral {

// ---------------------------------------------------------------------------
// Markov Chains, Mixing, Coupling, and MCMC
// Lectures 19, 20, 22 (MIT 6.856J): Markov Chains, UTS, Coupling
// ---------------------------------------------------------------------------
// A Markov chain is a sequence of random states where the next state
// depends only on the current state (memoryless property).
//
// Key concepts:
// - Transition matrix P: P[i][j] = Pr[X_{t+1} = j | X_t = i]
// - Stationary distribution pi: pi * P = pi
// - Mixing time: number of steps to get close to stationary distribution
// - Coupling: technique to bound mixing time

struct MarkovChain {
    int n;  // number of states
    std::vector<std::vector<double>> P; // transition matrix

    explicit MarkovChain(int n_) : n(n_), P(n_, std::vector<double>(n_, 0.0)) {}

    void set_transition(int from, int to, double prob) {
        P[from][to] = prob;
    }

    // Verify rows sum to 1
    bool valid() const {
        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) sum += P[i][j];
            if (std::abs(sum - 1.0) > 1e-9) return false;
        }
        return true;
    }

    // Compute stationary distribution by power iteration
    std::vector<double> stationary_distribution(int max_iter = 1000) const {
        std::vector<double> pi(n, 1.0 / n);
        for (int iter = 0; iter < max_iter; ++iter) {
            std::vector<double> pi_new(n, 0.0);
            for (int j = 0; j < n; ++j) {
                for (int i = 0; i < n; ++i) {
                    pi_new[j] += pi[i] * P[i][j];
                }
            }
            double diff = 0.0;
            for (int i = 0; i < n; ++i) {
                diff += std::abs(pi_new[i] - pi[i]);
            }
            pi = pi_new;
            if (diff < 1e-12) break;
        }
        return pi;
    }

    // Simulate the chain for num_steps from initial state
    std::vector<int> simulate(int start, int num_steps, unsigned seed = 42) const {
        std::mt19937 rng(seed);
        std::uniform_real_distribution<double> uniform(0.0, 1.0);

        std::vector<int> trajectory(num_steps + 1);
        trajectory[0] = start;

        for (int t = 0; t < num_steps; ++t) {
            int current = trajectory[t];
            double r = uniform(rng);
            double cumulative = 0.0;
            for (int j = 0; j < n; ++j) {
                cumulative += P[current][j];
                if (r <= cumulative) {
                    trajectory[t + 1] = j;
                    break;
                }
            }
        }
        return trajectory;
    }

    // Compute P^k (matrix power) via repeated squaring
    std::vector<std::vector<double>> matrix_power(int k) const {
        // Start with identity
        std::vector<std::vector<double>> result(n, std::vector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) result[i][i] = 1.0;

        std::vector<std::vector<double>> base = P;
        while (k > 0) {
            if (k & 1) {
                // result = result * base
                std::vector<std::vector<double>> temp(n, std::vector<double>(n, 0.0));
                for (int i = 0; i < n; ++i) {
                    for (int j = 0; j < n; ++j) {
                        for (int l = 0; l < n; ++l) {
                            temp[i][j] += result[i][l] * base[l][j];
                        }
                    }
                }
                result = temp;
            }
            // base = base * base
            std::vector<std::vector<double>> temp(n, std::vector<double>(n, 0.0));
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    for (int l = 0; l < n; ++l) {
                        temp[i][j] += base[i][l] * base[l][j];
                    }
                }
            }
            base = temp;
            k >>= 1;
        }
        return result;
    }

    // Total variation distance after t steps from initial distribution
    double tv_distance(const std::vector<double>& init, int t) const {
        auto Pt = matrix_power(t);
        auto pi = stationary_distribution();

        std::vector<double> dist_now(n, 0.0);
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i < n; ++i) {
                dist_now[j] += init[i] * Pt[i][j];
            }
        }

        double tv = 0.0;
        for (int i = 0; i < n; ++i) {
            tv += std::abs(dist_now[i] - pi[i]);
        }
        return tv / 2.0;
    }

    // Estimate mixing time: find t such that TV distance < epsilon
    int mixing_time(double epsilon = 0.01, int max_t = 10000) const {
        std::vector<double> init(n, 1.0 / n);
        for (int t = 1; t <= max_t; ++t) {
            if (tv_distance(init, t) < epsilon) return t;
        }
        return max_t;
    }
};

// ---------------------------------------------------------------------------
// Coupling
// ---------------------------------------------------------------------------
// A coupling of two copies of the same Markov chain uses the SAME
// randomness so that once they meet, they stay together.
// The coupling time = meeting time bounds the mixing time.

struct CouplingResult {
    int meeting_time;
    std::vector<int> trajectory1;
    std::vector<int> trajectory2;
};

inline CouplingResult coupling_simulation(const MarkovChain& chain,
                                           int start1, int start2,
                                           int max_steps, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    int n = chain.n;
    int current1 = start1, current2 = start2;
    std::vector<int> traj1, traj2;
    traj1.push_back(current1);
    traj2.push_back(current2);

    int meeting = -1;

    for (int t = 0; t < max_steps; ++t) {
        // Use SAME random number for both chains
        double r = uniform(rng);

        // Transition chain 1
        double cum = 0.0;
        for (int j = 0; j < n; ++j) {
            cum += chain.P[current1][j];
            if (r <= cum) { current1 = j; break; }
        }

        // Transition chain 2 with same randomness
        cum = 0.0;
        for (int j = 0; j < n; ++j) {
            cum += chain.P[current2][j];
            if (r <= cum) { current2 = j; break; }
        }

        traj1.push_back(current1);
        traj2.push_back(current2);

        if (current1 == current2 && meeting == -1) {
            meeting = t + 1;
        }
    }

    return {meeting, traj1, traj2};
}

// ---------------------------------------------------------------------------
// Markov Chain Monte Carlo (MCMC)
// ---------------------------------------------------------------------------
// Use a Markov chain whose stationary distribution is the target
// distribution pi. Run the chain long enough, then sample.

// Metropolis-Hastings MCMC
// target[i] = unnormalized target probability for state i
// proposal[i][j] = probability of proposing state j from state i
inline std::vector<int> metropolis_hastings(const std::vector<double>& target,
                                             const std::vector<std::vector<double>>& proposal,
                                             int start, int num_samples,
                                             int burn_in = 1000,
                                             unsigned seed = 42) {
    int n = static_cast<int>(target.size());
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    // Normalize target
    double total = 0.0;
    for (double p : target) total += p;
    std::vector<double> pi(n);
    for (int i = 0; i < n; ++i) pi[i] = target[i] / total;

    std::vector<int> samples;
    samples.reserve(num_samples);
    int current = start;

    for (int t = 0; t < burn_in + num_samples; ++t) {
        // Propose
        double r = uniform(rng);
        int proposal_state = 0;
        double cum = 0.0;
        for (int j = 0; j < n; ++j) {
            cum += proposal[current][j];
            if (r <= cum) { proposal_state = j; break; }
        }

        // Acceptance ratio: (pi[j] * P[j->i]) / (pi[i] * P[i->j])
        double numer = pi[proposal_state] * proposal[proposal_state][current];
        double denom = pi[current] * proposal[current][proposal_state];
        double alpha = (denom > 1e-15) ? std::min(1.0, numer / denom) : 1.0;

        if (uniform(rng) < alpha) {
            current = proposal_state;
        }

        if (t >= burn_in) {
            samples.push_back(current);
        }
    }

    return samples;
}

// Gibbs sampler: special case of Metropolis-Hastings
// target: unnormalized target distribution over {0, ..., n-1}
// At each step, pick a random neighboring state and accept/reject
inline std::vector<int> gibbs_sampler(const std::vector<double>& target,
                                       int start, int num_samples,
                                       int burn_in = 1000,
                                       unsigned seed = 42) {
    int n = static_cast<int>(target.size());
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    double total = 0.0;
    for (double p : target) total += p;
    std::vector<double> pi(n);
    for (int i = 0; i < n; ++i) pi[i] = target[i] / total;

    std::vector<int> samples;
    samples.reserve(num_samples);
    int current = start;

    for (int t = 0; t < burn_in + num_samples; ++t) {
        // Pick a random neighbor
        int proposal = static_cast<int>(uniform(rng) * n) % n;
        if (proposal == current) proposal = (current + 1) % n;

        // Accept/reject
        double alpha = std::min(1.0, pi[proposal] / pi[current]);
        if (uniform(rng) < alpha) {
            current = proposal;
        }

        if (t >= burn_in) {
            samples.push_back(current);
        }
    }

    return samples;
}

// ---------------------------------------------------------------------------
// Uniform Tree Sampling (UTS) via Markov Chains
// Lecture 20 (MIT 6.856J): Uniform Tree Sampling
// ---------------------------------------------------------------------------
// Estimate the number of spanning trees in a graph using
// the Markov chain approach (Kirchhoff's matrix tree theorem
// gives the exact count, but MCMC provides an alternative).

// Simple random walk based spanning tree estimator:
// Count the number of times each edge is used in random walks
struct UTSResult {
    double estimated_count;
    double exact_count;  // from Kirchhoff if computed
    int num_samples;
};

// Compute the Laplacian matrix
inline std::vector<std::vector<double>> laplacian_matrix(int n,
    const std::vector<std::pair<int,int>>& edges) {
    std::vector<std::vector<double>> L(n, std::vector<double>(n, 0.0));
    for (auto [u, v] : edges) {
        L[u][u] += 1.0;
        L[v][v] += 1.0;
        L[u][v] -= 1.0;
        L[v][u] -= 1.0;
    }
    return L;
}

// Kirchhoff's Matrix Tree Theorem: number of spanning trees =
// any cofactor of the Laplacian (delete row i, col i, take determinant)
inline double spanning_tree_count_kirchhoff(int n,
    const std::vector<std::pair<int,int>>& edges) {
    auto L = laplacian_matrix(n, edges);

    // Delete last row and column
    int m = n - 1;
    std::vector<std::vector<double>> M(m, std::vector<double>(m));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            M[i][j] = L[i][j];
        }
    }

    // Gaussian elimination for determinant
    double det = 1.0;
    for (int i = 0; i < m; ++i) {
        // Find pivot
        int pivot = i;
        for (int k = i + 1; k < m; ++k) {
            if (std::abs(M[k][i]) > std::abs(M[pivot][i])) pivot = k;
        }
        std::swap(M[i], M[pivot]);
        if (std::abs(M[i][i]) < 1e-15) return 0.0;
        det *= M[i][i];
        for (int k = i + 1; k < m; ++k) {
            double factor = M[k][i] / M[i][i];
            for (int j = i; j < m; ++j) {
                M[k][j] -= factor * M[i][j];
            }
        }
    }
    return std::abs(det);
}

} // namespace ral
