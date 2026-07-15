#pragma once
// Markov and Chebyshev Inequalities: Demonstrations and applications

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace chapter3 {

// Verify Markov's inequality: Pr[Y >= t] <= E[Y]/t
void verify_markov(int n_samples = 100000, unsigned seed = 42) {
    std::mt19937 rng(seed);
    // Exponential distribution: heavy tail, good for demonstrating Markov
    std::exponential_distribution<double> dist(1.0);

    std::vector<double> samples(n_samples);
    double sum = 0;
    for (int i = 0; i < n_samples; ++i) {
        samples[i] = dist(rng);
        sum += samples[i];
    }
    double mean = sum / n_samples;

    std::cout << "Markov's Inequality Verification (Exponential(1))\n";
    std::cout << "  E[X] = " << mean << " (theoretical: 1.0)\n\n";
    std::cout << "  t      Pr[X>=t]    E[X]/t     Markov bound\n";

    for (double t : {0.5, 1.0, 2.0, 3.0, 5.0, 10.0}) {
        int count = 0;
        for (double s : samples)
            if (s >= t) ++count;
        double empirical = static_cast<double>(count) / n_samples;
        double bound = mean / t;
        double exact = std::exp(-t);  // for Exp(1)
        std::cout << "  " << std::setw(4) << t << "   "
                  << std::setw(8) << empirical << "   "
                  << std::setw(8) << bound << "   "
                  << std::setw(8) << exact << "\n";
    }
    std::cout << "\n";
}

// Verify Chebyshev's inequality: Pr[|X-mu| >= t*sigma] <= 1/t^2
void verify_chebyshev(int n_samples = 100000, unsigned seed = 42) {
    std::mt19937 rng(seed);
    // Use a distribution with known variance
    // Beta(2,5) has mean=2/7, variance=10/343
    std::gamma_distribution<double> g1(2.0, 1.0);
    std::gamma_distribution<double> g2(5.0, 1.0);

    std::vector<double> samples(n_samples);
    double sum = 0;
    for (int i = 0; i < n_samples; ++i) {
        double a = g1(rng), b = g2(rng);
        samples[i] = a / (a + b);
        sum += samples[i];
    }
    double mean = sum / n_samples;
    double var = 0;
    for (double s : samples)
        var += (s - mean) * (s - mean);
    var /= n_samples;
    double sigma = std::sqrt(var);

    std::cout << "Chebyshev's Inequality Verification (Beta(2,5))\n";
    std::cout << "  E[X] = " << mean << "  sigma = " << sigma << "\n\n";
    std::cout << "  t      Pr[|X-mu|>=t*sigma]   1/t^2\n";

    for (double t : {0.5, 1.0, 1.5, 2.0, 3.0, 5.0}) {
        int count = 0;
        for (double s : samples)
            if (std::abs(s - mean) >= t * sigma) ++count;
        double empirical = static_cast<double>(count) / n_samples;
        double bound = 1.0 / (t * t);
        std::cout << "  " << std::setw(4) << t << "        "
                  << std::setw(8) << empirical << "            "
                  << std::setw(8) << bound << "\n";
    }
    std::cout << "\n";
}

// Demonstrate second moment method
void demonstrate_second_moment(int n_samples = 100000, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(1, 100);

    // Indicator: X = 1 if random number is prime, 0 otherwise
    auto is_prime = [](int n) -> bool {
        if (n < 2) return false;
        if (n == 2 || n == 3) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;
        for (int i = 5; i * i <= n; i += 6)
            if (n % i == 0 || n % (i + 2) == 0) return false;
        return true;
    };

    std::vector<int> samples(n_samples);
    for (int i = 0; i < n_samples; ++i)
        samples[i] = is_prime(dist(rng)) ? 1 : 0;

    double mean = 0;
    for (int s : samples) mean += s;
    mean /= n_samples;

    double var = 0;
    for (int s : samples) var += (s - mean) * (s - mean);
    var /= n_samples;

    // Second moment method: Pr[X > 0] >= E[X]^2 / (E[X]^2 + Var(X))
    double bound = (mean * mean) / (mean * mean + var);

    int nonzero = 0;
    for (int s : samples)
        if (s > 0) ++nonzero;
    double actual = static_cast<double>(nonzero) / n_samples;

    std::cout << "Second Moment Method (Indicator of Primality in [1,100])\n";
    std::cout << "  E[X] = " << mean << "\n";
    std::cout << "  Var(X) = " << var << "\n";
    std::cout << "  Pr[X > 0] >= E[X]^2 / (E[X]^2 + Var(X)) = " << bound << "\n";
    std::cout << "  Actual Pr[X > 0] = " << actual << "\n";
    std::cout << "  Bound is valid? " << (bound <= actual + 1e-9 ? "Yes" : "No") << "\n\n";
}

void demonstrate_inequalities() {
    std::cout << "Tail Inequalities (Section 3.2)\n\n";
    verify_markov();
    verify_chebyshev();
    demonstrate_second_moment();
}

}  // namespace chapter3
