#pragma once

#include <cmath>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <numeric>

namespace chapter4 {

// Compute the Azuma-Hoeffding bound: Pr[|X_n - X_0| >= t] <= 2*exp(-t^2 / (2 * sum(c_i^2)))
// Parameters:
//   n         - number of steps in the martingale
//   max_diff  - uniform bound on |X_i - X_{i-1}| (all c_i = max_diff)
//   t         - deviation threshold
double azuma_hoeffding_bound(int n, double max_diff, double t) {
    double sum_c2 = n * max_diff * max_diff;
    return 2.0 * std::exp(-t * t / (2.0 * sum_c2));
}

// Compute the Azuma-Hoeffding bound with non-uniform differences
// c[i] is the bound on |X_i - X_{i-1}| for i = 1..n
double azuma_hoeffding_bound(const std::vector<double>& c, double t) {
    double sum_c2 = 0.0;
    for (double ci : c) {
        sum_c2 += ci * ci;
    }
    if (sum_c2 == 0.0) return (t > 0) ? 0.0 : 1.0;
    return 2.0 * std::exp(-t * t / (2.0 * sum_c2));
}

// ============================================================
// Application: Random Graph Chromatic Number
// ============================================================
// Build G(n, p) and count edges incrementally (Doob martingale).
// X_k = E[total edges | first k vertices revealed]
// Returns the final edge count.
struct GraphEdgeMartingale {
    int n;
    double p;
    std::mt19937& rng;
    std::uniform_real_distribution<double> dist;

    GraphEdgeMartingale(int n, double p, std::mt19937& rng)
        : n(n), p(p), rng(rng), dist(0.0, 1.0) {}

    // Simulate the graph and return the martingale sequence
    // X_k = expected number of edges given first k vertices
    std::vector<double> simulate() {
        std::vector<double> martingale(n + 1);
        int edges_so_far = 0;

        // X_0 = E[total edges] = C(n,2) * p
        double expected_total = 0.5 * n * (n - 1) * p;
        martingale[0] = expected_total;

        // Incrementally add vertices
        int total_edges = 0;
        for (int k = 1; k <= n; k++) {
            // When vertex k is added, it connects to k-1 existing vertices
            // Each edge appears with probability p
            for (int j = 0; j < k - 1; j++) {
                if (dist(rng) < p) {
                    total_edges++;
                }
            }
            // X_k = E[total | first k vertices] = edges among first k
            //        + expected edges from remaining n-k vertices
            double remaining_pairs = 0.5 * (n - k) * (n - k - 1) + (n - k) * k;
            // Actually: X_k = edges_among_first_k + E[remaining edges]
            // E[remaining] = C(n-k, 2)*p + (n-k)*k*p (edges within remaining + edges between)
            double erem = 0.5 * (n - k) * (n - k - 1) * p + (n - k) * k * p;
            martingale[k] = total_edges + erem;
        }

        return martingale;
    }
};

// ============================================================
// Application: Maximum of n uniform random variables
// ============================================================
// M_n = max(X_1, ..., X_n) where X_i ~ Uniform[0,1].
// Doob martingale: Z_k = E[M_n | X_1, ..., X_k].
// Bounded differences: |Z_k - Z_{k-1}| <= 1/n (changing one X_i
// changes the max by at most the spacing 1/n for the conditional expectation).

// Simulate the Doob martingale for max of n uniforms
std::vector<double> max_uniform_martingale(int n, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::vector<double> x(n);
    for (int i = 0; i < n; i++) x[i] = dist(rng);

    std::vector<double> martingale(n + 1);
    // Z_k = E[max(X_1,...,X_n) | X_1,...,X_k]
    // = max(x_1,...,x_k) * (k/n) + integral-based expectation of max of n-k uniforms * (1 - k/n)
    // More precisely: Z_k = max(x_1,...,x_k) when k = n.
    // For general k: Z_k = max(x_1,...,x_k) + (n-k)/n * E[Y] where Y is related to
    // the expected additional max from n-k uniform[0,1] random variables
    // conditioned on being >= max(x_1,...,x_k).
    // Actually, the correct formula:
    // Z_k = max(x_1,...,x_k) * Prob(at least one of remaining >= m) + ...
    // This is complex. Use the simpler bound: |Z_k - Z_{k-1}| <= 1.

    double running_max = 0.0;
    martingale[0] = 0.5; // E[max of n uniforms] = n/(n+1) for large n, but we compute E[max]

    // Numerical computation of conditional expectation
    for (int k = 1; k <= n; k++) {
        if (x[k - 1] > running_max) running_max = x[k - 1];

        // E[max(X_1,...,X_n) | X_1,...,X_k] where X_{k+1},...,X_n are i.i.d. Uniform[0,1]
        // = integral from 0 to 1 of max(running_max, y) * f_Y(y) dy where
        // we need the distribution of max of n-k uniforms.
        // If running_max = m, then:
        //   E[max(m, Y_1,...,Y_{n-k})] = m * Prob(all Y_i < m) + integral_m^1 y * (n-k)*y^{n-k-1} dy
        //                                = m * m^{n-k} + integral_m^1 (n-k) * y^{n-k} dy
        //                                = m^{n-k+1} + (n-k)/(n-k+1) * (1 - m^{n-k+1})
        //                                = m^{n-k+1} * (1 - (n-k)/(n-k+1)) + (n-k)/(n-k+1)
        //                                = m^{n-k+1}/(n-k+1) + (n-k)/(n-k+1)

        int remaining = n - k;
        double m = running_max;
        double m_power = std::pow(m, remaining + 1);
        double conditional_exp = m_power / (remaining + 1)
                               + static_cast<double>(remaining) / (remaining + 1);
        martingale[k] = conditional_exp;
    }

    return martingale;
}

void demonstrate_martingales() {
    std::mt19937 rng(42);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Martingales and the Azuma-Hoeffding Inequality\n";
    std::cout << std::string(58, '-') << "\n\n";

    // Part 1: Azuma-Hoeffding bound table
    std::cout << "Part 1: Azuma-Hoeffding Bound 2*exp(-t^2 / (2*n*c^2))\n";
    std::cout << "  (uniform differences c_i = c for all i)\n\n";
    std::cout << std::setw(6) << "n"
              << std::setw(8) << "c"
              << std::setw(8) << "t"
              << std::setw(14) << "Bound"
              << "\n";
    std::cout << std::string(36, '-') << "\n";

    struct AHTest { int n; double c; double t; };
    std::vector<AHTest> ah_tests = {
        {100, 1.0, 5.0}, {100, 1.0, 10.0}, {100, 1.0, 15.0},
        {100, 0.5, 5.0}, {100, 0.5, 10.0},
        {500, 1.0, 10.0}, {500, 1.0, 20.0}, {500, 1.0, 30.0},
        {1000, 1.0, 15.0}, {1000, 1.0, 30.0}, {1000, 1.0, 50.0},
        {1000, 0.1, 3.0}, {1000, 0.1, 5.0}
    };

    for (const auto& t : ah_tests) {
        double bound = azuma_hoeffding_bound(t.n, t.c, t.t);
        std::cout << std::setw(6) << t.n
                  << std::setw(8) << t.c
                  << std::setw(8) << t.t
                  << std::setw(14) << bound
                  << "\n";
    }

    // Part 2: Graph edge count martingale
    std::cout << "\n\nPart 2: Doob Martingale for Edge Count in G(n, p)\n\n";
    std::cout << "  n=50, p=0.3. E[edges] = C(50,2)*0.3 = 367.5\n\n";

    {
        GraphEdgeMartingale gem(50, 0.3, rng);
        auto mart = gem.simulate();

        std::cout << std::setw(6) << "k"
                  << std::setw(14) << "X_k"
                  << std::setw(14) << "|Delta|"
                  << "\n";
        std::cout << std::string(34, '-') << "\n";

        for (int k = 0; k <= 50; k += 5) {
            double delta = (k > 0) ? std::abs(mart[k] - mart[k - 1]) : 0.0;
            std::cout << std::setw(6) << k
                      << std::setw(14) << mart[k]
                      << std::setw(14) << delta
                      << "\n";
        }

        double total_diff = 0.0;
        for (int k = 1; k <= 50; k++) {
            total_diff += (mart[k] - mart[k - 1]) * (mart[k] - mart[k - 1]);
        }
        double X0 = mart[0];
        double Xn = mart[50];
        double actual_dev = std::abs(Xn - X0);
        double bound_1sigma = std::sqrt(2.0 * total_diff);

        std::cout << "\n  X_0 = " << X0 << ", X_n = " << Xn
                  << ", |X_n - X_0| = " << actual_dev << "\n";
        std::cout << "  sqrt(2 * sum(c_i^2)) = " << bound_1sigma << "\n";
        std::cout << "  Azuma-Hoeffding Pr[dev >= bound] <= 2*exp(-1) = "
                  << 2.0 / M_E << "\n";
    }

    // Part 3: Maximum of n uniforms martingale
    std::cout << "\n\nPart 3: Doob Martingale for Max of n Uniform[0,1]\n";
    std::cout << "  n=100, E[max] = n/(n+1) = " << 100.0 / 101.0 << "\n\n";

    {
        auto mart = max_uniform_martingale(100, rng);

        std::cout << std::setw(6) << "k"
                  << std::setw(14) << "Z_k"
                  << std::setw(14) << "|Delta|"
                  << "\n";
        std::cout << std::string(34, '-') << "\n";

        for (int k = 0; k <= 100; k += 10) {
            double delta = (k > 0) ? std::abs(mart[k] - mart[k - 1]) : 0.0;
            std::cout << std::setw(6) << k
                      << std::setw(14) << mart[k]
                      << std::setw(14) << delta
                      << "\n";
        }

        double max_delta = 0.0;
        for (int k = 1; k <= 100; k++) {
            double d = std::abs(mart[k] - mart[k - 1]);
            if (d > max_delta) max_delta = d;
        }
        std::cout << "\n  Max |Z_k - Z_{k-1}| = " << max_delta << "\n";
        std::cout << "  Azuma-Hoeffding bound for t=0.1: "
                  << azuma_hoeffding_bound(100, max_delta, 0.1) << "\n";
    }

    // Part 4: Non-uniform differences
    std::cout << "\n\nPart 4: Non-Uniform Differences\n";
    std::cout << "  c_i = 1/sqrt(i) for i = 1..n\n\n";

    {
        int n = 100;
        std::vector<double> c(n);
        for (int i = 0; i < n; i++) {
            c[i] = 1.0 / std::sqrt(static_cast<double>(i + 1));
        }

        double sum_c2 = 0.0;
        for (double ci : c) sum_c2 += ci * ci;

        std::cout << "  sum(c_i^2) = " << sum_c2 << "\n";
        std::cout << "  For comparison, uniform c_i = 1/sqrt(n): sum = "
                  << n * (1.0 / n) << " = 1.0\n\n";

        std::cout << std::setw(8) << "t"
                  << std::setw(14) << "Non-Uniform"
                  << std::setw(14) << "Uniform(c=1)"
                  << "\n";
        std::cout << std::string(36, '-') << "\n";

        for (double t = 0.5; t <= 3.0; t += 0.5) {
            double bound_nu = azuma_hoeffding_bound(c, t);
            double bound_u = azuma_hoeffding_bound(n, 1.0, t);
            std::cout << std::setw(8) << t
                      << std::setw(14) << bound_nu
                      << std::setw(14) << bound_u
                      << "\n";
        }
    }

    std::cout << "\nKey observations:\n";
    std::cout << "  - Azuma-Hoeffding provides exponential tails for martingales\n";
    std::cout << "  - The bound depends on sum(c_i^2), not just n\n";
    std::cout << "  - Non-uniform differences can yield tighter bounds\n";
    std::cout << "  - Doob martingales connect function concentration to martingale theory\n";
    std::cout << "  - Bounded differences condition is key: changing one input changes output by <= c_i\n";
}

} // namespace chapter4
