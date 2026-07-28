#include "test_framework.h"
#include "ral.h"
#include <random>
#include <set>
#include <cmath>
#include <numeric>

// =============================================================================
// 1. Reservoir Sampling
// =============================================================================

static void test_reservoir_sampling() {
    ral_test::TestRunner runner("Reservoir Sampling");
    std::mt19937 rng(42);

    runner.begin_test("k=0 returns empty");
    {
        std::vector<int> stream = {1, 2, 3, 4, 5};
        auto sample = ral::reservoir_sample(stream, 0, rng);
        TEST_ASSERT_EQ(runner, (int)sample.size(), 0);
    }
    runner.end_test();

    runner.begin_test("k>=n returns full stream");
    {
        std::vector<int> stream = {10, 20, 30, 40, 50};
        auto sample = ral::reservoir_sample(stream, 5, rng);
        TEST_ASSERT_EQ(runner, (int)sample.size(), 5);
        for (int i = 0; i < 5; ++i) {
            TEST_ASSERT_EQ(runner, sample[i], stream[i]);
        }
        auto sample2 = ral::reservoir_sample(stream, 100, rng);
        TEST_ASSERT_EQ(runner, (int)sample2.size(), 5);
    }
    runner.end_test();

    runner.begin_test("sample size is always k");
    {
        std::vector<int> stream(200);
        std::iota(stream.begin(), stream.end(), 0);
        for (int k : {1, 5, 10, 50, 200}) {
            auto sample = ral::reservoir_sample(stream, k, rng);
            TEST_ASSERT_EQ(runner, (int)sample.size(), k);
        }
    }
    runner.end_test();

    runner.begin_test("all elements come from the stream");
    {
        std::set<int> stream_set;
        std::vector<int> stream(100);
        for (int i = 0; i < 100; ++i) { stream[i] = i + 1000; stream_set.insert(i + 1000); }
        auto sample = ral::reservoir_sample(stream, 15, rng);
        for (int x : sample) {
            TEST_ASSERT_TRUE_MSG(runner, stream_set.count(x) > 0,
                "sampled element not in original stream");
        }
    }
    runner.end_test();

    runner.begin_test("statistical uniformity (5000 trials)");
    {
        int n = 100, k = 10, trials = 5000;
        std::vector<int> stream(n);
        std::iota(stream.begin(), stream.end(), 0);
        std::vector<int> freq(n, 0);

        for (int t = 0; t < trials; ++t) {
            auto sample = ral::reservoir_sample(stream, k, rng);
            for (int x : sample) freq[x]++;
        }

        double expected_prob = (double)k / n;
        bool all_close = true;
        for (int i = 0; i < n; ++i) {
            double observed = (double)freq[i] / trials;
            if (std::abs(observed - expected_prob) > 0.05) {
                all_close = false;
                break;
            }
        }
        TEST_ASSERT_TRUE_MSG(runner, all_close,
            "some element deviates from expected probability k/n by more than 0.05");
    }
    runner.end_test();

    runner.begin_test("weighted: weight-10 sampled ~10x more often than weight-1");
    {
        int n = 6, k = 3, trials = 5000;
        std::vector<int> stream = {0, 1, 2, 3, 4, 5};
        std::vector<double> weights = {1.0, 1.0, 10.0, 10.0, 10.0, 10.0};
        std::vector<int> freq(n, 0);

        for (int t = 0; t < trials; ++t) {
            auto sample = ral::weighted_reservoir_sample(stream, weights, k, rng);
            for (int x : sample) freq[x]++;
        }

        // Expected: P(elem i) ~ k * w_i / sum(weights)
        // sum(weights) = 42, k=3
        // weight-10: 3*10/42 ≈ 0.714, weight-1: 3*1/42 ≈ 0.071
        // So weight-10 elements should appear ~10x more often
        double total_weight = 42.0;
        double avg_high = (double)(freq[2] + freq[3] + freq[4] + freq[5]) / 4.0 / trials;
        double avg_low = (double)(freq[0] + freq[1]) / 2.0 / trials;
        double expected_high = k * 10.0 / total_weight;
        double expected_low = k * 1.0 / total_weight;
        TEST_ASSERT_NEAR(runner, avg_high, expected_high, 0.10);
        TEST_ASSERT_NEAR(runner, avg_low, expected_low, 0.10);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// =============================================================================
// 2. Randomized Rounding
// =============================================================================

static void test_randomized_rounding() {
    ral_test::TestRunner runner("Randomized Rounding");
    std::mt19937 rng(42);

    runner.begin_test("basic rounding is binary (0 or 1)");
    {
        // Trivial LP: 4 variables, all fractional=0.5, all constraints loose
        int n = 4;
        std::vector<double> lp_sol = {0.5, 0.5, 0.5, 0.5};
        std::vector<std::vector<double>> A = {{1.0, 1.0, 1.0, 1.0}};
        std::vector<double> b = {10.0};
        std::vector<double> c = {1.0, 2.0, 3.0, 4.0};

        auto result = ral::randomized_rounding_basic(lp_sol, A, b, c, rng);
        for (int x : result.rounded) {
            TEST_ASSERT_TRUE_MSG(runner, x == 0 || x == 1,
                "rounded value must be 0 or 1");
        }
    }
    runner.end_test();

    runner.begin_test("result has correct fields");
    {
        int n = 3;
        std::vector<double> lp_sol = {0.4, 0.6, 0.8};
        std::vector<std::vector<double>> A = {{1.0, 1.0, 1.0}};
        std::vector<double> b = {5.0};
        std::vector<double> c = {10.0, 20.0, 30.0};

        auto result = ral::randomized_rounding_basic(lp_sol, A, b, c, rng);
        double expected_lp_obj = 10.0 * 0.4 + 20.0 * 0.6 + 30.0 * 0.8;
        TEST_ASSERT_NEAR(runner, result.lp_objective, expected_lp_obj, 1e-9);
        TEST_ASSERT_TRUE_MSG(runner, result.rounded_objective >= 0,
            "rounded objective should be non-negative");
    }
    runner.end_test();

    runner.begin_test("trivial LP: all constraints loose");
    {
        int n = 3;
        std::vector<double> lp_sol = {0.5, 0.5, 0.5};
        std::vector<std::vector<double>> A = {{1.0, 0.0, 0.0},
                                              {0.0, 1.0, 0.0},
                                              {0.0, 0.0, 1.0}};
        std::vector<double> b = {5.0, 5.0, 5.0};
        std::vector<double> c = {1.0, 1.0, 1.0};

        int feasible_count = 0;
        int trials = 500;
        for (int t = 0; t < trials; ++t) {
            auto result = ral::randomized_rounding_basic(lp_sol, A, b, c, rng);
            if (result.satisfies_constraints) feasible_count++;
        }
        // With b[i]=5 and each x_i <= 1, all must be feasible
        TEST_ASSERT_EQ(runner, feasible_count, trials);
    }
    runner.end_test();

    runner.begin_test("best-of returns feasible more often than single trial");
    {
        // Tight constraint: x0 + x1 <= 1 with both lp_sol=0.5
        // Single trial: feasible ~50% of the time (both 0, or one 0 one 1)
        std::vector<double> lp_sol = {0.5, 0.5};
        std::vector<std::vector<double>> A = {{1.0, 1.0}};
        std::vector<double> b = {1.0};
        std::vector<double> c = {1.0, 1.0};

        int single_feasible = 0;
        int best_of_feasible = 0;
        int trials = 2000;
        for (int t = 0; t < trials; ++t) {
            auto s = ral::randomized_rounding_basic(lp_sol, A, b, c, rng);
            if (s.satisfies_constraints) single_feasible++;

            auto b_res = ral::randomized_rounding_best_of(lp_sol, A, b, c, 10, rng);
            if (b_res.satisfies_constraints) best_of_feasible++;
        }

        // best-of-10 should be feasible more often (it picks the best of 10)
        TEST_ASSERT_GT(runner, best_of_feasible, single_feasible);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// =============================================================================
// 3. Pollard's Rho
// =============================================================================

static void test_pollard_rho() {
    ral_test::TestRunner runner("Pollard's Rho");
    std::mt19937 rng(42);

    runner.begin_test("factor divides n for known composites");
    {
        long long f1 = ral::pollard_rho_factor(91, rng);
        TEST_ASSERT_TRUE_MSG(runner, 91 % f1 == 0, "factor must divide 91");
        TEST_ASSERT_TRUE_MSG(runner, f1 > 1 && f1 < 91, "factor must be non-trivial");

        long long f2 = ral::pollard_rho_factor(123456789, rng);
        TEST_ASSERT_TRUE_MSG(runner, 123456789 % f2 == 0, "factor must divide 123456789");
        TEST_ASSERT_TRUE_MSG(runner, f2 > 1 && f2 < 123456789, "factor must be non-trivial");
    }
    runner.end_test();

    runner.begin_test("full factorization of 100 = 2^2 * 5^2");
    {
        auto fp = ral::prime_factorization(100, rng);
        TEST_ASSERT_EQ(runner, (int)fp.size(), 2);
        TEST_ASSERT_EQ(runner, fp[0].prime, 2);
        TEST_ASSERT_EQ(runner, fp[0].exponent, 2);
        TEST_ASSERT_EQ(runner, fp[1].prime, 5);
        TEST_ASSERT_EQ(runner, fp[1].exponent, 2);
    }
    runner.end_test();

    runner.begin_test("primes return themselves");
    {
        auto fp = ral::prime_factorization(997, rng);
        TEST_ASSERT_EQ(runner, (int)fp.size(), 1);
        TEST_ASSERT_EQ(runner, fp[0].prime, 997);
        TEST_ASSERT_EQ(runner, fp[0].exponent, 1);
    }
    runner.end_test();

    runner.begin_test("product of all factors equals n");
    {
        std::vector<long long> test_nums = {91, 100, 123456789, 600851475143LL};
        for (long long n : test_nums) {
            auto fp = ral::prime_factorization(n, rng);
            long long product = 1;
            for (auto& [p, e] : fp) {
                for (int i = 0; i < e; ++i) product *= p;
            }
            TEST_ASSERT_EQ(runner, product, n);
        }
    }
    runner.end_test();

    runner.begin_test("600851475143 = 71*839*1471*6857");
    {
        auto fp = ral::prime_factorization(600851475143LL, rng);
        TEST_ASSERT_EQ(runner, (int)fp.size(), 4);
        TEST_ASSERT_EQ(runner, fp[0].prime, 71);
        TEST_ASSERT_EQ(runner, fp[1].prime, 839);
        TEST_ASSERT_EQ(runner, fp[2].prime, 1471);
        TEST_ASSERT_EQ(runner, fp[3].prime, 6857);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// =============================================================================
// 4. Welzl's Minimum Enclosing Circle
// =============================================================================

static void test_welzl_mec() {
    ral_test::TestRunner runner("Welzl MEC");
    std::mt19937 rng(42);

    runner.begin_test("single point: radius=0, center=point");
    {
        std::vector<ral::Point2D_mec> pts = {{3.0, 4.0}};
        auto c = ral::welzl_mec(pts, rng);
        TEST_ASSERT_NEAR(runner, c.center.x, 3.0, 1e-9);
        TEST_ASSERT_NEAR(runner, c.center.y, 4.0, 1e-9);
        TEST_ASSERT_NEAR(runner, c.radius, 0.0, 1e-9);
    }
    runner.end_test();

    runner.begin_test("two points: radius = half distance");
    {
        std::vector<ral::Point2D_mec> pts = {{0.0, 0.0}, {6.0, 8.0}};
        auto c = ral::welzl_mec(pts, rng);
        TEST_ASSERT_NEAR(runner, c.radius, 5.0, 1e-9);
        TEST_ASSERT_NEAR(runner, c.center.x, 3.0, 1e-9);
        TEST_ASSERT_NEAR(runner, c.center.y, 4.0, 1e-9);
    }
    runner.end_test();

    runner.begin_test("all points contained in circle");
    {
        std::vector<ral::Point2D_mec> pts = {{0, 0}, {4, 0}, {2, 3.46}, {-1, 2}, {5, -1}};
        auto c = ral::welzl_mec(pts, rng);
        for (auto& p : pts) {
            TEST_ASSERT_TRUE_MSG(runner, c.contains(p), "point not in circle");
        }
    }
    runner.end_test();

    runner.begin_test("equilateral triangle: radius matches circumradius");
    {
        // Equilateral triangle with side length 4
        double side = 4.0;
        std::vector<ral::Point2D_mec> pts = {{0, 0}, {4, 0}, {2, 2.0 * std::sqrt(3.0)}};
        auto c = ral::welzl_mec(pts, rng);
        double expected_r = side / std::sqrt(3.0);
        TEST_ASSERT_NEAR(runner, c.radius, expected_r, 1e-6);
    }
    runner.end_test();

    runner.begin_test("unit square: radius = sqrt(2)/2");
    {
        std::vector<ral::Point2D_mec> pts = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        auto c = ral::welzl_mec(pts, rng);
        TEST_ASSERT_NEAR(runner, c.radius, std::sqrt(2.0) / 2.0, 1e-9);
        TEST_ASSERT_NEAR(runner, c.center.x, 0.5, 1e-9);
        TEST_ASSERT_NEAR(runner, c.center.y, 0.5, 1e-9);
    }
    runner.end_test();

    runner.begin_test("brute-force verification: matches min over all triples");
    {
        std::mt19937 rng2(42);
        std::uniform_real_distribution<double> dist(-10.0, 10.0);
        std::vector<ral::Point2D_mec> pts(12);
        for (auto& p : pts) p = {dist(rng2), dist(rng2)};

        auto c = ral::welzl_mec(pts, rng2);

        // Brute-force: min radius over all pairs and triples that cover all points
        double min_r = 1e18;
        int sz = (int)pts.size();
        for (int i = 0; i < sz; ++i) {
            for (int j = i + 1; j < sz; ++j) {
                auto c2 = ral::circle_from_2(pts[i], pts[j]);
                bool covers = true;
                for (auto& p : pts) covers &= c2.contains(p);
                if (covers && c2.radius < min_r) min_r = c2.radius;
            }
        }
        for (int i = 0; i < sz; ++i) {
            for (int j = i + 1; j < sz; ++j) {
                for (int k = j + 1; k < sz; ++k) {
                    auto c3 = ral::circle_from_3(pts[i], pts[j], pts[k]);
                    bool covers = true;
                    for (auto& p : pts) covers &= c3.contains(p);
                    if (covers && c3.radius < min_r) min_r = c3.radius;
                }
            }
        }
        TEST_ASSERT_NEAR(runner, c.radius, min_r, 1e-6);
    }
    runner.end_test();

    runner.begin_test("recursive wrapper matches iterative");
    {
        std::vector<ral::Point2D_mec> pts = {{0, 0}, {3, 0}, {1.5, 2.0}, {0.5, -1.0}};
        auto c1 = ral::welzl_mec(pts, rng);
        auto c2 = ral::welzl_mec_recursive_wrapper(pts, rng);
        TEST_ASSERT_NEAR(runner, c1.radius, c2.radius, 1e-9);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// =============================================================================
// 5. Locality-Sensitive Hashing
// =============================================================================

static void test_lsh() {
    ral_test::TestRunner runner("LSH");
    std::mt19937 rng(42);

    runner.begin_test("SimHash: same vector hashes to same signature");
    {
        ral::SimHash sh(10, 8, rng);
        std::vector<double> v = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
        auto h1 = sh.hash(v);
        auto h2 = sh.hash(v);
        TEST_ASSERT_EQ(runner, (int)h1.size(), 8);
        TEST_ASSERT_TRUE_MSG(runner, h1 == h2, "same vector must produce same hash");
    }
    runner.end_test();

    runner.begin_test("SimHash: different vectors produce different signatures");
    {
        ral::SimHash sh(10, 32, rng);
        std::vector<double> v1(10), v2(10);
        std::normal_distribution<double> norm(0.0, 1.0);
        for (int i = 0; i < 10; ++i) { v1[i] = norm(rng); v2[i] = norm(rng); }
        auto h1 = sh.hash(v1);
        auto h2 = sh.hash(v2);
        int hamming = ral::SimHash::hamming_distance(h1, h2);
        TEST_ASSERT_GT(runner, hamming, 0);
    }
    runner.end_test();

    runner.begin_test("HyperplaneLSH: same vector hashes to same bucket");
    {
        ral::HyperplaneLSH hlsh(8, 16, rng);
        std::vector<double> v = {1.5, -0.3, 2.7, 0.0, 1.1, -2.2, 3.3, 0.5};
        auto h1 = hlsh.hash(v);
        auto h2 = hlsh.hash(v);
        TEST_ASSERT_TRUE_MSG(runner, h1 == h2, "same vector must produce same hash");
    }
    runner.end_test();

    runner.begin_test("LSHIndex: insert and query finds exact match");
    {
        std::mt19937 rng2(42);
        int dim = 10;
        ral::LSHIndex index(dim, 16, rng2);

        std::vector<double> pt = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
        index.insert(pt, 0);

        auto results = index.query(pt, 10);
        TEST_ASSERT_TRUE_MSG(runner, !results.empty(), "query for inserted point should find it");
        TEST_ASSERT_EQ(runner, results[0].index, 0);
        TEST_ASSERT_NEAR(runner, results[0].distance, 0.0, 1e-9);
    }
    runner.end_test();

    runner.begin_test("E2LSH: same vector hashes to same signature");
    {
        ral::E2LSH e2lsh(8, 16, 2.0, rng);
        std::vector<double> v = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        auto h1 = e2lsh.hash(v);
        auto h2 = e2lsh.hash(v);
        TEST_ASSERT_TRUE_MSG(runner, h1 == h2, "same vector must produce same E2LSH hash");
    }
    runner.end_test();

    runner.begin_test("E2LSH: insert items, query returns nearest items");
    {
        std::mt19937 rng2(42);
        int dim = 5;
        int k = 16;
        double w = 2.0;
        ral::E2LSH e2lsh(dim, k, w, rng2);

        // Insert a cluster of points near (0,...,0)
        std::normal_distribution<double> close_norm(0.0, 0.05);
        std::vector<std::vector<double>> near_pts(10);
        for (int i = 0; i < 10; ++i) {
            near_pts[i].resize(dim);
            for (int j = 0; j < dim; ++j) near_pts[i][j] = close_norm(rng2);
        }

        // All near points should hash to the same signature (bucket)
        auto sig0 = e2lsh.hash(near_pts[0]);
        bool all_same = true;
        for (int i = 1; i < 10; ++i) {
            if (e2lsh.hash(near_pts[i]) != sig0) { all_same = false; break; }
        }
        TEST_ASSERT_TRUE_MSG(runner, all_same,
            "very close points should hash to the same E2LSH bucket");
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// =============================================================================
// Main
// =============================================================================

int main() {
    test_reservoir_sampling();
    test_randomized_rounding();
    test_pollard_rho();
    test_welzl_mec();
    test_lsh();

    bool all_pass = ral_test::run_all_suites();
    return all_pass ? 0 : 1;
}
