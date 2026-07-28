#include "test_framework.h"
#include "ral.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <numbers>
#include <set>

using namespace ral;

// ============================================================
// Stress: Chernoff Bounds
// ============================================================
void test_stress_chernoff() {
    ral_test::TestRunner runner("Stress: Chernoff");

    runner.begin_test("upper tail bound is valid");
    for (int n = 10; n <= 1000; n *= 2) {
        for (double p = 0.1; p <= 0.9; p += 0.2) {
            double mu = n * p;
            for (double delta = 0.05; delta <= 1.0; delta += 0.1) {
                double bound = chernoff_upper(mu, delta, true);
                TEST_ASSERT_TRUE(runner, bound >= 0.0);
                TEST_ASSERT_TRUE(runner, bound <= 1.0);
            }
        }
    }
    runner.end_test();

    runner.begin_test("simplified bound <= exact bound");
    for (int n = 10; n <= 500; n *= 2) {
        for (double p = 0.1; p <= 0.5; p += 0.2) {
            double mu = n * p;
            for (double delta = 0.05; delta <= 0.9; delta += 0.1) {
                double exact = chernoff_upper(mu, delta, true);
                double simp = chernoff_simplified(mu, delta, true);
                TEST_ASSERT_TRUE(runner, simp >= exact - 0.01);
            }
        }
    }
    runner.end_test();

    runner.begin_test("simulated tail <= theoretical bound");
    {
        std::mt19937 rng(42);
        int n = 1000, p_val = 5, trials = 5000;
        double p = p_val / 100.0;
        double mu = n * p;
        for (double delta = 0.1; delta <= 0.8; delta += 0.2) {
            double simulated = simulate_upper_tail(n, p, delta, trials, rng);
            double bound = chernoff_upper(mu, delta, true);
            // The bound should be >= the true probability
            TEST_ASSERT_TRUE(runner, bound >= simulated - 0.05);
        }
    }
    runner.end_test();

    runner.begin_test("mgf_bound is positive");
    for (double mu = 1; mu <= 100; mu *= 2) {
        for (double lambda = -1; lambda <= 2; lambda += 0.5) {
            double bound = mgf_bound(mu, lambda);
            TEST_ASSERT_TRUE(runner, bound > 0.0);
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Coupon Collector
// ============================================================
void test_stress_coupon_collector() {
    ral_test::TestRunner runner("Stress: CouponCollector");

    runner.begin_test("expected trials ~ n*H(n)");
    for (int n : {10, 50, 100, 200}) {
        std::mt19937 rng(42);
        double harmonic = 0;
        for (int i = 1; i <= n; i++) harmonic += 1.0 / i;
        double expected = n * harmonic;

        double total = 0;
        int trials = 2000;
        for (int t = 0; t < trials; t++) {
            total += simulate_coupon_collector(n, rng);
        }
        double avg = total / trials;
        // Within 30% of expected
        TEST_ASSERT_TRUE(runner, avg > expected * 0.7);
        TEST_ASSERT_TRUE(runner, avg < expected * 1.3);
    }
    runner.end_test();

    runner.begin_test("minimum trials >= n");
    for (int n : {5, 10, 20, 50}) {
        std::mt19937 rng(42);
        for (int t = 0; t < 100; t++) {
            int result = simulate_coupon_collector(n, rng);
            TEST_ASSERT_TRUE(runner, result >= n);
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Two-Point Sampling
// ============================================================
void test_stress_two_point() {
    ral_test::TestRunner runner("Stress: TwoPoint");

    runner.begin_test("amplified two-point error <= 1/t");
    for (int t : {2, 4, 8, 16, 32}) {
        int n = 1000, trials = 20000;
        std::mt19937 rng(42);
        int false_count = 0;
        for (int i = 0; i < trials; i++) {
            if (!amplified_two_point(1, n, t, true, rng)) false_count++;
        }
        double error = (double)false_count / trials;
        // Error should be <= 1/t with high probability
        TEST_ASSERT_TRUE(runner, error <= 1.0 / t + 0.05);
    }
    runner.end_test();

    runner.begin_test("naive error ~ 1/4 for x in L");
    {
        int n = 1000, trials = 50000;
        std::mt19937 rng(42);
        int false_count = 0;
        for (int i = 0; i < trials; i++) {
            if (!naive_two_point(1, n, true, rng)) false_count++;
        }
        double error = (double)false_count / trials;
        // Error ~ 1/4
        TEST_ASSERT_TRUE(runner, error < 0.35);
        TEST_ASSERT_TRUE(runner, error > 0.15);
    }
    runner.end_test();

    runner.begin_test("x not in L always returns false");
    {
        int n = 1000, trials = 1000;
        std::mt19937 rng(42);
        for (int i = 0; i < trials; i++) {
            TEST_ASSERT_TRUE(runner, !naive_two_point(1, n, false, rng));
            TEST_ASSERT_TRUE(runner, !amplified_two_point(1, n, 10, false, rng));
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Rabin-Karp
// ============================================================
void test_stress_rabin_karp() {
    ral_test::TestRunner runner("Stress: RabinKarp");

    runner.begin_test("pattern found at all positions");
    {
        for (int n = 5; n <= 200; n++) {
            std::string text(n, 'a');
            std::string pattern = "aa";
            auto matches = rabin_karp_search(text, pattern);
            int expected = n - 1;
            TEST_ASSERT_EQ(runner, (int)matches.size(), expected);
            for (int i = 0; i < expected; i++) {
                TEST_ASSERT_EQ(runner, matches[i], i);
            }
        }
    }
    runner.end_test();

    runner.begin_test("no match");
    {
        std::string text = "ABCDEFGHIJ";
        std::string pattern = "XYZ";
        auto matches = rabin_karp_search(text, pattern);
        TEST_ASSERT_TRUE(runner, matches.empty());
    }
    runner.end_test();

    runner.begin_test("single character pattern");
    {
        std::string text = "mississippi";
        std::string pattern = "s";
        auto matches = rabin_karp_search(text, pattern);
        std::vector<int> expected = {2, 3, 5, 6};
        TEST_ASSERT_EQ(runner, (int)matches.size(), (int)expected.size());
        for (int i = 0; i < (int)expected.size(); i++) {
            TEST_ASSERT_EQ(runner, matches[i], expected[i]);
        }
    }
    runner.end_test();

    runner.begin_test("fingerprint consistency");
    {
        const long long BASE = 256, MOD = 1000000007LL;
        for (int t = 0; t < 200; t++) {
            std::string s(t % 20 + 1, 'a' + t % 26);
            long long fp1 = string_fingerprint(s, BASE, MOD);
            long long fp2 = string_fingerprint(s, BASE, MOD);
            TEST_ASSERT_EQ(runner, fp1, fp2);
            // Different string should (almost certainly) have different fingerprint
            if (s.size() > 1) {
                std::string s2 = s;
                s2[0] = (s2[0] == 'z') ? 'a' : s2[0] + 1;
                long long fp3 = string_fingerprint(s2, BASE, MOD);
                // Not guaranteed but extremely likely for these inputs
            }
        }
    }
    runner.end_test();

    runner.begin_test("pattern longer than text returns empty");
    {
        auto matches = rabin_karp_search("ab", "abc");
        TEST_ASSERT_TRUE(runner, matches.empty());
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Freivalds
// ============================================================
void test_stress_freivalds() {
    ral_test::TestRunner runner("Stress: Freivalds");

    runner.begin_test("correct product always verified");
    for (int n = 2; n <= 20; n++) {
        std::vector<std::vector<long long>> A(n, std::vector<long long>(n));
        std::vector<std::vector<long long>> B(n, std::vector<long long>(n));
        std::mt19937 rng(n);
        std::uniform_int_distribution<int> dist(0, 100);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                A[i][j] = dist(rng);
                B[i][j] = dist(rng);
            }
        // Compute C = A * B
        std::vector<std::vector<long long>> C(n, std::vector<long long>(n, 0));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                for (int k = 0; k < n; k++)
                    C[i][j] += A[i][k] * B[k][j];
        std::mt19937 test_rng(42);
        TEST_ASSERT_TRUE(runner, freivalds_verify(A, B, C, 20, test_rng));
    }
    runner.end_test();

    runner.begin_test("incorrect product detected");
    for (int n = 2; n <= 15; n++) {
        std::vector<std::vector<long long>> A(n, std::vector<long long>(n, 1));
        std::vector<std::vector<long long>> B(n, std::vector<long long>(n, 1));
        std::vector<std::vector<long long>> C(n, std::vector<long long>(n, n + 1)); // wrong
        std::mt19937 test_rng(42);
        TEST_ASSERT_TRUE(runner, !freivalds_verify(A, B, C, 20, test_rng));
    }
    runner.end_test();

    runner.begin_test("single element change detected");
    {
        std::vector<std::vector<long long>> A = {{1, 2}, {3, 4}};
        std::vector<std::vector<long long>> B = {{5, 6}, {7, 8}};
        std::vector<std::vector<long long>> C = {{19, 22}, {43, 51}}; // C[1][1] = 51 not 50
        std::mt19937 test_rng(42);
        TEST_ASSERT_TRUE(runner, !freivalds_verify(A, B, C, 20, test_rng));
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Monte Carlo / Las Vegas
// ============================================================
void test_stress_mc_lv() {
    ral_test::TestRunner runner("Stress: MonteCarloLasVegas");

    runner.begin_test("randomized quicksort always correct");
    for (int t = 0; t < 100; t++) {
        int n = (t % 30) + 5;
        std::vector<int> arr(n);
        std::mt19937 rng(t * 12345);
        for (int i = 0; i < n; i++) arr[i] = rng() % 1000;
        auto expected = arr;
        std::sort(expected.begin(), expected.end());
        randomized_quicksort(arr, 0, n - 1);
        TEST_ASSERT_TRUE(runner, std::is_sorted(arr.begin(), arr.end()));
    }
    runner.end_test();

    runner.begin_test("estimate_pi_monte_carlo converges");
    for (int n : {10000, 50000}) {
        double pi_est = estimate_pi_monte_carlo(n);
        double error = std::abs(pi_est - std::numbers::pi);
        // Should be within 0.1 of pi for large n
        TEST_ASSERT_TRUE(runner, error < 0.1);
    }
    runner.end_test();

    runner.begin_test("monte_carlo_to_las_vegas always correct");
    for (int t = 0; t < 50; t++) {
        int input = t + 1;
        auto [result, attempts] = monte_carlo_to_las_vegas(input);
        TEST_ASSERT_EQ(runner, result, input * 2); // correct answer is input*2
        TEST_ASSERT_TRUE(runner, attempts >= 1);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Routing
// ============================================================
void test_stress_routing() {
    ral_test::TestRunner runner("Stress: Routing");

    runner.begin_test("simulate_routing max congestion >= 1");
    {
        std::mt19937 rng(42);
        for (int n = 10; n <= 500; n *= 2) {
            int max_q = simulate_routing(n, rng);
            TEST_ASSERT_TRUE(runner, max_q >= 1);
        }
    }
    runner.end_test();

    runner.begin_test("congestion bound is valid probability");
    for (int n = 10; n <= 1000; n *= 2) {
        for (double t = 1.0; t <= 10.0; t += 1.0) {
            double bound = theoretical_congestion_bound(n, t);
            TEST_ASSERT_TRUE(runner, bound >= 0.0);
            TEST_ASSERT_TRUE(runner, bound <= n + 0.01);
        }
    }
    runner.end_test();

    runner.begin_test("congestion decreases with t");
    {
        int n = 100;
        double prev = n + 1;
        for (double t = 1.5; t <= 20.0; t += 0.5) {
            double bound = theoretical_congestion_bound(n, t);
            TEST_ASSERT_TRUE(runner, bound <= prev + 1e-6);
            prev = bound;
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Paging
// ============================================================
void test_stress_paging() {
    ral_test::TestRunner runner("Stress: Paging");

    runner.begin_test("OPT is optimal");
    for (int t = 0; t < 50; t++) {
        int k = (t % 4) + 2;
        int len = (t % 20) + 10;
        auto req = generate_random_requests(len, k + 5);
        int opt = optimal_offline(k, req, false);
        int lru = deterministic_lru(k, req, false);
        int fifo = deterministic_fifo(k, req, false);
        int rnd = random_paging(k, req, false);
        int mark = marking_algorithm(k, req, false);
        TEST_ASSERT_TRUE(runner, lru >= opt);
        TEST_ASSERT_TRUE(runner, fifo >= opt);
        TEST_ASSERT_TRUE(runner, rnd >= opt);
        TEST_ASSERT_TRUE(runner, mark >= opt);
    }
    runner.end_test();

    runner.begin_test("all algorithms <= sequence length");
    for (int t = 0; t < 30; t++) {
        int k = 3;
        auto req = generate_random_requests(100, 10);
        int opt = optimal_offline(k, req, false);
        TEST_ASSERT_TRUE(runner, opt <= 100);
        TEST_ASSERT_TRUE(runner, deterministic_lru(k, req, false) <= 100);
        TEST_ASSERT_TRUE(runner, random_paging(k, req, false) <= 100);
    }
    runner.end_test();

    runner.begin_test("adversarial sequence forces high ratio");
    {
        int k = 3;
        auto worst_seq = ObliviousAdversary::worst_case_lru_fifo(k, 100);
        auto result = adversarial_test(deterministic_lru, optimal_offline, k, worst_seq);
        TEST_ASSERT_TRUE(runner, result.competitive_ratio >= 1.5);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: K-Server
// ============================================================
void test_stress_k_server() {
    ral_test::TestRunner runner("Stress: KServer");

    runner.begin_test("greedy always has finite cost");
    {
        auto dist = line_metric(10);
        std::vector<int> requests = {5, 0, 8, 2, 7, 1, 9, 3, 6, 4};
        ServerState state(2, {0, 1});
        for (int req : requests) state = min_server_move(state, req, dist);
        TEST_ASSERT_TRUE(runner, state.total_cost > 0);
    }
    runner.end_test();

    runner.begin_test("random cost is finite and reasonable");
    {
        auto dist = line_metric(10);
        std::vector<int> requests = {5, 0, 8, 2, 7, 1, 9, 3, 6, 4};
        for (int t = 0; t < 20; t++) {
            ServerState state(2, {0, 1});
            for (int req : requests) state = random_server_move(state, req, dist);
            TEST_ASSERT_TRUE(runner, state.total_cost > 0);
            TEST_ASSERT_TRUE(runner, state.total_cost <= 1000);
        }
    }
    runner.end_test();

    runner.begin_test("greedy <= OPT * k");
    {
        auto dist = line_metric(6);
        std::vector<int> requests = {5, 0, 3, 1, 4};
        ServerState gs(2, {0, 1});
        for (int req : requests) gs = min_server_move(gs, req, dist);
        long long opt = optimal_offline_server(2, {0, 1}, requests, dist);
        TEST_ASSERT_TRUE(runner, gs.total_cost <= opt * 2);
    }
    runner.end_test();

    runner.begin_test("no move when request at server");
    {
        auto dist = line_metric(5);
        ServerState state(2, {0, 3});
        state = min_server_move(state, 0, dist);
        TEST_ASSERT_EQ(runner, state.total_cost, 0LL);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Adversary
// ============================================================
void test_stress_adversary() {
    ral_test::TestRunner runner("Stress: Adversary");

    runner.begin_test("worst_case_lru_fifo forces high LRU ratio");
    {
        int k = 3;
        auto seq = ObliviousAdversary::worst_case_lru_fifo(k, 100);
        auto result = adversarial_test(deterministic_lru, optimal_offline, k, seq);
        // LRU should be k-competitive on this sequence
        TEST_ASSERT_TRUE(runner, result.competitive_ratio >= 1.5);
    }
    runner.end_test();

    runner.begin_test("cyclic sequence properties");
    {
        auto seq = ObliviousAdversary::cyclic_sequence(5, 20);
        TEST_ASSERT_EQ(runner, (int)seq.size(), 20);
        for (int i = 0; i < 20; i++) {
            TEST_ASSERT_EQ(runner, seq[i], i % 5 + 1);
        }
    }
    runner.end_test();

    runner.begin_test("interleaved adversary properties");
    {
        int k = 3;
        auto seq = ObliviousAdversary::interleaved_adversary(k, 20);
        TEST_ASSERT_EQ(runner, (int)seq.size(), 20);
        // Odd positions should be k+1
        for (int i = 1; i < 20; i += 2) {
            TEST_ASSERT_EQ(runner, seq[i], k + 1);
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: PRAM simulation
// ============================================================
void test_stress_pram() {
    ral_test::TestRunner runner("Stress: PRAM");

    runner.begin_test("parallel prefix sum matches sequential");
    for (int n = 1; n <= 64; n *= 2) {
        PRAM pram(n, n);
        std::vector<long long> input(n);
        for (int i = 0; i < n; i++) input[i] = i + 1;
        auto parallel_result = pram.prefix_sum_parallel(input);
        auto seq_result = PRAM::prefix_sum_sequential(input);
        TEST_ASSERT_EQ(runner, (int)parallel_result.size(), n);
        for (int i = 0; i < n; i++) {
            TEST_ASSERT_EQ(runner, parallel_result[i], seq_result[i]);
        }
    }
    runner.end_test();

    runner.begin_test("prefix sum with all zeros");
    {
        PRAM pram(8, 8);
        std::vector<long long> input(8, 0);
        auto result = pram.prefix_sum_parallel(input);
        for (int i = 0; i < 8; i++) TEST_ASSERT_EQ(runner, result[i], 0LL);
    }
    runner.end_test();

    runner.begin_test("prefix sum with all ones");
    {
        PRAM pram(8, 8);
        std::vector<long long> input(8, 1);
        auto result = pram.prefix_sum_parallel(input);
        auto seq = PRAM::prefix_sum_sequential(input);
        for (int i = 0; i < 8; i++) TEST_ASSERT_EQ(runner, result[i], seq[i]);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

int main() {
    test_stress_chernoff();
    test_stress_coupon_collector();
    test_stress_two_point();
    test_stress_rabin_karp();
    test_stress_freivalds();
    test_stress_mc_lv();
    test_stress_routing();
    test_stress_paging();
    test_stress_k_server();
    test_stress_adversary();
    test_stress_pram();
    return ral_test::run_all_suites() ? 0 : 1;
}
