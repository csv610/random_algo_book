// example_azuma_inequality.cpp
// Demonstrates the Azuma-Hoeffding inequality for martingale difference
// sequences. If (X_0, X_1, ..., X_n) is a martingale with bounded
// differences |X_i - X_{i-1}| <= c_i, then:
//
//   Pr[|X_n - X_0| >= t] <= 2 * exp(-t^2 / (2 * sum(c_i^2)))
//
// This example shows:
// 1. Computing the bound for various parameters
// 2. A Doob martingale for edge counts in G(n,p)
// 3. A Doob martingale for max of n Uniform[0,1]
// 4. Non-uniform difference bounds

#include "ral/martingales.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace ral;

int main() {
    std::mt19937 rng(42);
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "=== Azuma-Hoeffding Inequality ===\n\n";

    // Part 1: Bound table
    std::cout << "Pr[|X_n - X_0| >= t] <= 2*exp(-t^2 / (2*n*c^2))\n\n";
    std::cout << std::setw(6) << "n"
              << std::setw(8) << "c"
              << std::setw(8) << "t"
              << std::setw(14) << "Bound"
              << "\n";
    std::cout << std::string(36, '-') << "\n";

    struct AHTest { int n; double c; double t; };
    for (const auto& [n, c, t] : std::vector<AHTest>{
        {100, 1.0, 5.0}, {100, 1.0, 10.0}, {100, 0.5, 5.0},
        {500, 1.0, 10.0}, {500, 1.0, 20.0}, {500, 1.0, 30.0},
        {1000, 1.0, 15.0}, {1000, 1.0, 30.0}, {1000, 0.1, 3.0}
    }) {
        double bound = azuma_hoeffding_bound(n, c, t);
        std::cout << std::setw(6) << n
                  << std::setw(8) << c
                  << std::setw(8) << t
                  << std::setw(14) << bound
                  << "\n";
    }

    // Part 2: Doob martingale for edge count in G(n,p)
    std::cout << "\n\nDoob Martingale: Edge Count in G(50, 0.3)\n";
    std::cout << "E[edges] = C(50,2)*0.3 = " << 0.5 * 50 * 49 * 0.3 << "\n\n";

    {
        GraphEdgeMartingale gem(50, 0.3, rng);
        auto mart = gem.simulate();

        std::cout << std::setw(6) << "k"
                  << std::setw(14) << "X_k"
                  << std::setw(14) << "|Delta|"
                  << "\n";
        std::cout << std::string(34, '-') << "\n";

        for (int k = 0; k <= 50; k += 10) {
            double delta = (k > 0) ? std::abs(mart[k] - mart[k - 1]) : 0.0;
            std::cout << std::setw(6) << k
                      << std::setw(14) << mart[k]
                      << std::setw(14) << delta
                      << "\n";
        }

        double sum_c2 = 0.0;
        for (int k = 1; k <= 50; k++)
            sum_c2 += (mart[k] - mart[k - 1]) * (mart[k] - mart[k - 1]);

        std::cout << "\n  X_0 = " << mart[0] << ", X_50 = " << mart[50]
                  << ", |X_50 - X_0| = " << std::abs(mart[50] - mart[0]) << "\n";
        std::cout << "  sqrt(2 * sum(c_i^2)) = " << std::sqrt(2.0 * sum_c2) << "\n";
    }

    // Part 3: Max of n uniforms martingale
    std::cout << "\n\nDoob Martingale: Max of 100 Uniform[0,1]\n";
    std::cout << "E[max] = n/(n+1) = " << 100.0 / 101.0 << "\n\n";

    {
        auto mart = max_uniform_martingale(100, rng);

        std::cout << std::setw(6) << "k"
                  << std::setw(14) << "Z_k"
                  << std::setw(14) << "|Delta|"
                  << "\n";
        std::cout << std::string(34, '-') << "\n";

        for (int k = 0; k <= 100; k += 20) {
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
        std::cout << "  Azuma-Hoeffding Pr[dev >= 0.1] <= "
                  << azuma_hoeffding_bound(100, max_delta, 0.1) << "\n";
    }

    // Part 4: Non-uniform differences
    std::cout << "\n\nNon-Uniform Differences: c_i = 1/sqrt(i)\n";
    {
        int n = 100;
        std::vector<double> c(n);
        for (int i = 0; i < n; i++)
            c[i] = 1.0 / std::sqrt(static_cast<double>(i + 1));

        double sum_c2 = 0.0;
        for (double ci : c) sum_c2 += ci * ci;

        std::cout << "  sum(c_i^2) = " << sum_c2
                  << " (vs uniform c=1/sqrt(n): " << n * (1.0 / n) << ")\n\n";

        std::cout << std::setw(8) << "t"
                  << std::setw(14) << "Non-Uniform"
                  << std::setw(14) << "Uniform(c=1)"
                  << "\n";
        std::cout << std::string(36, '-') << "\n";

        for (double t = 0.5; t <= 3.0; t += 0.5) {
            std::cout << std::setw(8) << t
                      << std::setw(14) << azuma_hoeffding_bound(c, t)
                      << std::setw(14) << azuma_hoeffding_bound(n, 1.0, t)
                      << "\n";
        }
    }

    return 0;
}
