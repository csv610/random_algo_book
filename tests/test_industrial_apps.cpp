#include "test_framework.h"
#include "ral.h"
#include <random>
#include <vector>
#include <string>
#include <cmath>
#include <numeric>
#include <algorithm>

// =============================================================================
// 1. Streaming Analytics
// =============================================================================
void test_streaming_analytics() {
    ral_test::TestRunner runner("Streaming Analytics");

    // --- StreamingCountMinSketch ---
    {
        std::mt19937 rng(42);

        runner.begin_test("CMS dimensions positive");
        ral::StreamingCountMinSketch cms(0.01, 0.01, rng);
        TEST_ASSERT_GT(runner, cms.width(), 0);
        TEST_ASSERT_GT(runner, cms.depth(), 0);
        runner.end_test();

        runner.begin_test("CMS single item estimate equals 1");
        std::mt19937 rng2(42);
        ral::StreamingCountMinSketch cms2(0.01, 0.01, rng2);
        cms2.update("itemA");
        TEST_ASSERT_EQ(runner, cms2.estimate("itemA"), 1);
        runner.end_test();

        runner.begin_test("CMS multiple updates exact count");
        std::mt19937 rng3(42);
        ral::StreamingCountMinSketch cms3(0.01, 0.01, rng3);
        for (int i = 0; i < 50; i++) cms3.update("target");
        TEST_ASSERT_EQ(runner, cms3.estimate("target"), 50);
        runner.end_test();

        runner.begin_test("CMS never underestimates");
        std::mt19937 rng4(42);
        ral::StreamingCountMinSketch cms4(0.01, 0.01, rng4);
        for (int i = 0; i < 100; i++) cms4.update("heavy");
        for (int i = 0; i < 200; i++) cms4.update("light");
        TEST_ASSERT_GE(runner, cms4.estimate("heavy"), 100);
        runner.end_test();

        runner.begin_test("CMS error bound for heavy stream");
        std::mt19937 rng5(42);
        double eps = 0.01;
        ral::StreamingCountMinSketch cms5(eps, 0.01, rng5);
        for (int i = 0; i < 10000; i++) cms5.update("freq");
        for (int i = 0; i < 10000; i++) {
            cms5.update("rare_" + std::to_string(i));
        }
        long long est = cms5.estimate("freq");
        long long true_freq = 10000;
        long long error = est - true_freq;
        TEST_ASSERT_LT(runner, error, static_cast<long long>(eps * cms5.total_count()));
        runner.end_test();

        runner.begin_test("CMS estimate_zero non-negative");
        std::mt19937 rng6(42);
        ral::StreamingCountMinSketch cms6(0.01, 0.01, rng6);
        for (int i = 0; i < 50; i++) cms6.update("present");
        TEST_ASSERT_GE(runner, cms6.estimate_zero(), 0);
        runner.end_test();
    }

    // --- FlajoletMartin ---
    {
        runner.begin_test("FM stream count increments");
        std::mt19937 rng(42);
        ral::FlajoletMartin fm(64, rng);
        TEST_ASSERT_EQ(runner, fm.stream_count(), 0);
        fm.update("a");
        TEST_ASSERT_EQ(runner, fm.stream_count(), 1);
        fm.update("b");
        TEST_ASSERT_EQ(runner, fm.stream_count(), 2);
        runner.end_test();

        runner.begin_test("FM estimate positive");
        std::mt19937 rng2(42);
        ral::FlajoletMartin fm2(64, rng2);
        for (int i = 0; i < 100; i++) fm2.update("item_" + std::to_string(i));
        TEST_ASSERT_GT(runner, fm2.estimate(), 0.0);
        runner.end_test();

        runner.begin_test("FM many distinct items within 10x");
        std::mt19937 rng3(42);
        ral::FlajoletMartin fm3(256, rng3);
        int n_distinct = 500;
        for (int i = 0; i < n_distinct; i++) fm3.update("distinct_" + std::to_string(i));
        double est = fm3.estimate();
        TEST_ASSERT_TRUE_MSG(runner, est > 0.0,
            "estimate must be positive");
        TEST_ASSERT_TRUE_MSG(runner, est / n_distinct < 10.0 && n_distinct / est < 10.0,
            "estimate within 10x of true count");
        runner.end_test();

        runner.begin_test("FM identical items estimate near 1");
        std::mt19937 rng4(42);
        ral::FlajoletMartin fm4(64, rng4);
        for (int i = 0; i < 1000; i++) fm4.update("same");
        double est2 = fm4.estimate();
        TEST_ASSERT_NEAR(runner, est2, 1.0, 5.0);
        runner.end_test();
    }

    // --- MisraGries ---
    {
        runner.begin_test("MG single item in candidates");
        ral::MisraGries mg(10);
        mg.update("only");
        auto cands = mg.candidates();
        TEST_ASSERT_EQ(runner, static_cast<int>(cands.size()), 1);
        TEST_ASSERT_TRUE_MSG(runner, cands[0].item == "only", "candidate should be 'only'");
        runner.end_test();

        runner.begin_test("MG heavy hitter in candidates");
        ral::MisraGries mg2(5);
        for (int i = 0; i < 100; i++) mg2.update("heavy");
        for (int i = 0; i < 10; i++) mg2.update("light");
        auto cands2 = mg2.candidates();
        bool found = false;
        for (auto& c : cands2) {
            if (c.item == "heavy") found = true;
        }
        TEST_ASSERT_TRUE_MSG(runner, found, "heavy hitter must be in candidates");
        runner.end_test();

        runner.begin_test("MG total_count matches updates");
        ral::MisraGries mg3(3);
        for (int i = 0; i < 50; i++) mg3.update("x");
        TEST_ASSERT_EQ(runner, mg3.total_count(), 50);
        runner.end_test();

        runner.begin_test("MG k=1 gives zero candidates");
        ral::MisraGries mg4(1);
        for (int i = 0; i < 20; i++) mg4.update("item");
        auto cands3 = mg4.candidates();
        TEST_ASSERT_EQ(runner, static_cast<int>(cands3.size()), 0);
        runner.end_test();
    }

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// =============================================================================
// 2. Simulated Annealing
// =============================================================================
void test_simulated_annealing() {
    ral_test::TestRunner runner("Simulated Annealing");

    runner.begin_test("SA constant cost function best equals initial cost");
    {
        std::mt19937 rng(42);
        std::vector<int> initial = {1, 2, 3};
        auto constant_cost = [](const std::vector<int>&) -> double { return 42.0; };
        auto neighbor = [](const std::vector<int>& s, std::mt19937& r) { return s; };
        auto result = ral::simulated_annealing(initial, constant_cost, neighbor,
                                                100.0, 0.99, 0.01, 10, rng);
        TEST_ASSERT_NEAR(runner, result.best_cost, 42.0, 0.001);
    }
    runner.end_test();

    runner.begin_test("SA descending chain best stays at initial");
    {
        std::mt19937 rng(42);
        std::vector<int> initial = {5};
        auto cost = [](const std::vector<int>& s) -> double {
            return static_cast<double>(s[0]);
        };
        auto neighbor = [](const std::vector<int>& s, std::mt19937& r) -> std::vector<int> {
            return {s[0] + 1};
        };
        auto result = ral::simulated_annealing(initial, cost, neighbor,
                                                1.0, 0.9, 0.01, 5, rng);
        TEST_ASSERT_NEAR(runner, result.best_cost, 5.0, 0.001);
    }
    runner.end_test();

    runner.begin_test("knapsack all zeros cost is 0");
    {
        ral::KnapsackInstance inst;
        inst.weights = {10, 20, 30};
        inst.values = {60, 100, 120};
        inst.capacity = 50;
        std::vector<int> solution = {0, 0, 0};
        TEST_ASSERT_NEAR(runner, ral::knapsack_cost(solution, inst), 0.0, 0.001);
    }
    runner.end_test();

    runner.begin_test("knapsack full feasible has negative cost");
    {
        ral::KnapsackInstance inst;
        inst.weights = {10, 20};
        inst.values = {60, 100};
        inst.capacity = 50;
        std::vector<int> solution = {1, 1};
        double cost = ral::knapsack_cost(solution, inst);
        TEST_ASSERT_TRUE_MSG(runner, cost < 0.0, "feasible knapsack cost should be negative");
    }
    runner.end_test();

    runner.begin_test("knapsack over-capacity returns large positive");
    {
        ral::KnapsackInstance inst;
        inst.weights = {30, 30, 30};
        inst.values = {10, 10, 10};
        inst.capacity = 10;
        std::vector<int> solution = {1, 1, 1};
        double cost = ral::knapsack_cost(solution, inst);
        TEST_ASSERT_GT(runner, cost, 1e8);
    }
    runner.end_test();

    runner.begin_test("TSP identity tour on square perimeter");
    {
        ral::TSPInstance inst;
        inst.x = {0.0, 1.0, 1.0, 0.0};
        inst.y = {0.0, 0.0, 1.0, 1.0};
        std::vector<int> tour = {0, 1, 2, 3};
        double cost = ral::tsp_cost(tour, inst);
        TEST_ASSERT_NEAR(runner, cost, 4.0, 1e-9);
    }
    runner.end_test();

    runner.begin_test("SA minimize x^2 near 0");
    {
        std::mt19937 rng(42);
        std::vector<int> initial = {7};
        auto cost = [](const std::vector<int>& s) -> double {
            double x = static_cast<double>(s[0]);
            return x * x;
        };
        auto neighbor = [](const std::vector<int>& s, std::mt19937& r) -> std::vector<int> {
            std::uniform_int_distribution<int> dist(-1, 1);
            int step = dist(r);
            int val = s[0] + step;
            val = std::clamp(val, -10, 10);
            return {val};
        };
        auto result = ral::simulated_annealing(initial, cost, neighbor,
                                                50.0, 0.99, 0.01, 20, rng);
        TEST_ASSERT_LT(runner, result.best_cost, 2.0);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// =============================================================================
// 3. Differential Privacy
// =============================================================================
void test_differential_privacy() {
    ral_test::TestRunner runner("Differential Privacy");

    runner.begin_test("RR collect returns correct count");
    {
        std::mt19937 rng(42);
        std::vector<bool> answers = {true, false, true, true, false};
        auto result = ral::collect_randomized_responses(answers, 1.0, rng);
        TEST_ASSERT_EQ(runner, static_cast<int>(result.raw_responses.size()), 5);
    }
    runner.end_test();

    runner.begin_test("RR responses count equals input size");
    {
        std::mt19937 rng(42);
        std::vector<bool> answers(100, true);
        auto result = ral::collect_randomized_responses(answers, 0.8, rng);
        TEST_ASSERT_EQ(runner, static_cast<int>(result.raw_responses.size()), 100);
        TEST_ASSERT_EQ(runner, result.total, 100);
    }
    runner.end_test();

    runner.begin_test("RR p=1.0 all truthful");
    {
        std::mt19937 rng(42);
        std::vector<bool> answers = {true, false, true, false, true, false};
        auto result = ral::collect_randomized_responses(answers, 1.0, rng);
        bool all_match = true;
        for (size_t i = 0; i < answers.size(); i++) {
            if (result.raw_responses[i] != answers[i]) all_match = false;
        }
        TEST_ASSERT_TRUE_MSG(runner, all_match, "all responses should be truthful with p=1.0");
        runner.end_test();
    }

    runner.begin_test("estimate_proportion p=1.0 equals exact");
    {
        std::mt19937 rng(42);
        int n = 1000;
        std::vector<bool> answers(n);
        int true_yes = 300;
        for (int i = 0; i < n; i++) answers[i] = (i < true_yes);
        auto result = ral::collect_randomized_responses(answers, 1.0, rng);
        double estimated = ral::estimate_proportion(result.raw_responses, 1.0);
        double exact = static_cast<double>(true_yes) / n;
        TEST_ASSERT_NEAR(runner, estimated, exact, 0.01);
    }
    runner.end_test();

    runner.begin_test("estimate_proportion result in [0,1]");
    {
        std::mt19937 rng(42);
        int n = 500;
        std::vector<bool> answers(n);
        std::uniform_real_distribution<double> unif(0.0, 1.0);
        for (int i = 0; i < n; i++) answers[i] = (unif(rng) < 0.3);
        auto result = ral::collect_randomized_responses(answers, 0.75, rng);
        double est = ral::estimate_proportion(result.raw_responses, 0.75);
        TEST_ASSERT_GE(runner, est, -0.5);
        TEST_ASSERT_LE(runner, est, 1.5);
    }
    runner.end_test();

    runner.begin_test("laplace_mechanism average approximates true value");
    {
        std::mt19937 rng(42);
        double true_val = 100.0;
        double sum = 0.0;
        int trials = 5000;
        for (int i = 0; i < trials; i++) {
            sum += ral::laplace_mechanism(true_val, 1.0, 1.0, rng);
        }
        double avg = sum / trials;
        TEST_ASSERT_NEAR(runner, avg, true_val, 1.0);
    }
    runner.end_test();

    runner.begin_test("private_mean is finite");
    {
        std::mt19937 rng(42);
        std::vector<double> values = {10.0, 20.0, 30.0, 40.0, 50.0};
        double pm = ral::private_mean(values, 100.0, 1.0, rng);
        TEST_ASSERT_TRUE(runner, std::isfinite(pm));
    }
    runner.end_test();

    runner.begin_test("private_histogram bucket count matches input");
    {
        std::mt19937 rng(42);
        int num_buckets = 5;
        std::vector<int> data = {0, 1, 2, 3, 4, 0, 1, 2};
        auto hist = ral::private_histogram(data, num_buckets, 1.0, rng);
        TEST_ASSERT_EQ(runner, static_cast<int>(hist.size()), num_buckets);
        bool all_finite = true;
        for (double v : hist) {
            if (!std::isfinite(v)) all_finite = false;
        }
        TEST_ASSERT_TRUE_MSG(runner, all_finite, "all buckets must be finite");
    }
    runner.end_test();

    runner.begin_test("higher epsilon lower average error");
    {
        std::mt19937 rng(42);
        std::vector<double> values = {5.0, 10.0, 15.0, 20.0, 25.0};
        double true_mean = 15.0;
        int trials = 2000;

        double err_low_eps = 0.0;
        for (int i = 0; i < trials; i++) {
            err_low_eps += std::abs(ral::private_mean(values, 50.0, 0.5, rng) - true_mean);
        }
        err_low_eps /= trials;

        double err_high_eps = 0.0;
        for (int i = 0; i < trials; i++) {
            err_high_eps += std::abs(ral::private_mean(values, 50.0, 5.0, rng) - true_mean);
        }
        err_high_eps /= trials;

        TEST_ASSERT_TRUE_MSG(runner, err_high_eps < err_low_eps,
            "higher epsilon should produce lower average error");
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

int main() {
    test_streaming_analytics();
    test_simulated_annealing();
    test_differential_privacy();

    bool all_pass = ral_test::run_all_suites();
    return all_pass ? 0 : 1;
}
