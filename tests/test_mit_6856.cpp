#include "test_framework.h"
#include <set>
#include "ral/stable_marriage.h"
#include "ral/game_tree.h"
#include "ral/dnf_counting.h"
#include "ral/expander.h"
#include "ral/markov_chain.h"
#include "ral/conditional_expectation.h"
#include "ral/randomized_shortest_paths.h"
#include "ral/max_sat.h"
#include "ral/trapezoidal.h"

using namespace ral;

// ============================================================
// Stable Marriage
// ============================================================
void test_stable_marriage() {
    ral_test::TestRunner runner("Stable Marriage");

    runner.begin_test("Gale-Shapley produces valid matching");
    {
        for (int n : {2, 3, 5, 8, 10}) {
            auto result = randomized_gale_shapley(n, 42);
            TEST_ASSERT_TRUE(runner, result.stable);
            TEST_ASSERT_EQ(runner, static_cast<int>(result.man_partner.size()), n);
        }
    }
    runner.end_test();

    runner.begin_test("Stability verified on random instances");
    {
        for (int trial = 0; trial < 20; ++trial) {
            int n = 5 + trial % 5;
            std::mt19937 rng(trial * 7);
            std::vector<std::vector<int>> pref(n, std::vector<int>(n));
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) pref[i][j] = j;
                std::shuffle(pref[i].begin(), pref[i].end(), rng);
            }
            auto result = gale_shapley(pref);
            TEST_ASSERT_TRUE(runner, result.stable);
            TEST_ASSERT_TRUE(runner, verify_stable_matching(pref, result.man_partner));
        }
    }
    runner.end_test();

    runner.begin_test("All partners distinct");
    {
        for (int n : {3, 6, 10}) {
            auto result = randomized_gale_shapley(n, 123);
            std::set<int> men(result.man_partner.begin(), result.man_partner.end());
            TEST_ASSERT_EQ(runner, static_cast<int>(men.size()), n);
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Game Tree
// ============================================================
void test_game_tree() {
    ral_test::TestRunner runner("Game Tree");

    runner.begin_test("Deterministic evaluation correctness");
    {
        GameTree tree(0);
        int n1 = tree.add_node(NodeType::MIN_NODE);
        int n2 = tree.add_node(NodeType::MIN_NODE);
        tree.add_child(0, n1);
        tree.add_child(0, n2);
        tree.add_child(n1, tree.add_node(NodeType::LEAF, 0.3));
        tree.add_child(n1, tree.add_node(NodeType::LEAF, 0.7));
        tree.add_child(n2, tree.add_node(NodeType::LEAF, 0.5));
        tree.add_child(n2, tree.add_node(NodeType::LEAF, 0.9));
        // MIN picks min of children: min(0.3, 0.7) = 0.3, min(0.5, 0.9) = 0.5
        // MAX picks max of those: max(0.3, 0.5) = 0.5
        TEST_ASSERT_NEAR(runner, deterministic_evaluate(tree, 0), 0.5, 1e-9);
    }
    runner.end_test();

    runner.begin_test("Randomized matches deterministic");
    {
        auto tree = GameTree::build_binary_tree(6, 42);
        double det = deterministic_evaluate(tree, 0);
        for (int trial = 0; trial < 10; ++trial) {
            std::mt19937 rng(trial);
            double rand_val = randomized_evaluate(tree, 0, rng);
            TEST_ASSERT_NEAR(runner, rand_val, det, 1e-9);
        }
    }
    runner.end_test();

    runner.begin_test("Monte Carlo converges");
    {
        auto tree = GameTree::build_binary_tree(4, 42);
        double det = deterministic_evaluate(tree, 0);
        auto mc = monte_carlo_evaluate(tree, 10000, 42);
        TEST_ASSERT_TRUE(runner, std::abs(mc.estimate - det) < 0.15);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// DNF Counting
// ============================================================
void test_dnf_counting() {
    ral_test::TestRunner runner("DNF Counting");

    runner.begin_test("Exact count correct");
    {
        std::vector<DNFClause> clauses = {{{1, 2}}, {{-1, 3}}, {{2, -3}}};
        double exact = exact_dnf_count(3, clauses);
        TEST_ASSERT_EQ(runner, static_cast<int>(exact), 5);
    }
    runner.end_test();

    runner.begin_test("Karp-Luby approximation");
    {
        std::vector<DNFClause> clauses = {{{1, 2}}, {{-1, 3}}, {{2, -3}}};
        for (double eps : {0.3, 0.2, 0.1}) {
            auto result = karp_luby_dnf(3, clauses, eps, 42);
            TEST_ASSERT_TRUE(runner, result.estimate > 0);
            TEST_ASSERT_TRUE(runner, result.estimate <= 8.0);
        }
    }
    runner.end_test();

    runner.begin_test("Empty DNF");
    {
        std::vector<DNFClause> empty;
        auto result = karp_luby_dnf(3, empty, 0.1, 42);
        TEST_ASSERT_EQ(runner, static_cast<int>(result.estimate), 0);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Expander Graphs
// ============================================================
void test_expander() {
    ral_test::TestRunner runner("Expander");

    runner.begin_test("Random regular graph is d-regular");
    {
        auto G = random_regular_graph(20, 3, 42);
        TEST_ASSERT_EQ(runner, G.n, 20);
        TEST_ASSERT_EQ(runner, G.d, 3);
        for (int v = 0; v < G.n; ++v) {
            TEST_ASSERT_EQ(runner, static_cast<int>(G.adj[v].size()), 3);
        }
    }
    runner.end_test();

    runner.begin_test("Spectral gap is positive");
    {
        auto G = random_regular_graph(30, 4, 42);
        int gap = G.spectral_gap(50);
        TEST_ASSERT_TRUE(runner, gap >= 0);
    }
    runner.end_test();

    runner.begin_test("Expansion ratio");
    {
        auto G = random_regular_graph(20, 3, 42);
        std::vector<bool> set1(20, false);
        for (int i = 0; i < 3; ++i) set1[i] = true;
        double exp = G.expansion_ratio(set1);
        TEST_ASSERT_TRUE(runner, exp > 0);
    }
    runner.end_test();

    runner.begin_test("PRG output");
    {
        auto G = random_regular_graph(16, 3, 42);
        std::vector<bool> seed = {true, false, true};
        auto prg = expander_prg(G, seed, 30);
        TEST_ASSERT_EQ(runner, static_cast<int>(prg.output.size()), 30);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Markov Chains
// ============================================================
void test_markov_chain() {
    ral_test::TestRunner runner("Markov Chain");

    runner.begin_test("Stationary distribution sums to 1");
    {
        MarkovChain chain(3);
        chain.set_transition(0, 0, 0.5); chain.set_transition(0, 1, 0.5);
        chain.set_transition(1, 1, 0.3); chain.set_transition(1, 2, 0.7);
        chain.set_transition(2, 2, 0.6); chain.set_transition(2, 0, 0.4);
        auto pi = chain.stationary_distribution();
        double sum = 0;
        for (double p : pi) sum += p;
        TEST_ASSERT_NEAR(runner, sum, 1.0, 1e-6);
    }
    runner.end_test();

    runner.begin_test("Coupling meets");
    {
        MarkovChain chain(4);
        chain.set_transition(0, 0, 0.3); chain.set_transition(0, 1, 0.7);
        chain.set_transition(1, 0, 0.2); chain.set_transition(1, 1, 0.3);
        chain.set_transition(1, 2, 0.5);
        chain.set_transition(2, 2, 0.4); chain.set_transition(2, 3, 0.6);
        chain.set_transition(3, 3, 0.8); chain.set_transition(3, 0, 0.2);
        auto coup = coupling_simulation(chain, 0, 3, 200, 42);
        TEST_ASSERT_TRUE(runner, coup.meeting_time > 0);
        TEST_ASSERT_TRUE(runner, coup.meeting_time <= 200);
    }
    runner.end_test();

    runner.begin_test("TV distance decreases");
    {
        MarkovChain chain(3);
        chain.set_transition(0, 0, 0.5); chain.set_transition(0, 1, 0.5);
        chain.set_transition(1, 1, 0.3); chain.set_transition(1, 2, 0.7);
        chain.set_transition(2, 2, 0.6); chain.set_transition(2, 0, 0.4);
        std::vector<double> init = {1.0, 0.0, 0.0};
        double tv1 = chain.tv_distance(init, 1);
        double tv10 = chain.tv_distance(init, 10);
        TEST_ASSERT_TRUE(runner, tv10 < tv1);
    }
    runner.end_test();

    runner.begin_test("Metropolis-Hastings sampling");
    {
        std::vector<double> target = {1.0, 3.0, 2.0};
        std::vector<std::vector<double>> proposal(3, std::vector<double>(3, 1.0/3.0));
        auto samples = metropolis_hastings(target, proposal, 0, 5000, 1000, 42);
        TEST_ASSERT_EQ(runner, static_cast<int>(samples.size()), 5000);
        // Check state 1 is sampled most
        std::vector<int> counts(3, 0);
        for (int s : samples) counts[s]++;
        TEST_ASSERT_TRUE(runner, counts[1] > counts[0]);
    }
    runner.end_test();

    runner.begin_test("Kirchhoff spanning tree count");
    {
        // K4 has 16 spanning trees
        std::vector<std::pair<int,int>> edges = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
        double count = spanning_tree_count_kirchhoff(4, edges);
        TEST_ASSERT_EQ(runner, static_cast<int>(count), 16);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Conditional Expectation / Derandomized MAX-SAT
// ============================================================
void test_conditional_expectation() {
    ral_test::TestRunner runner("Conditional Expectation");

    runner.begin_test("Derandomized MAX-SAT");
    {
        std::vector<CNFClause> clauses = {
            {{1, 2}}, {{-1, 3}}, {{2, -3}}
        };
        auto result = derandomized_max_sat(clauses, 3);
        TEST_ASSERT_TRUE(runner, result.clauses_satisfied >= 2);
        TEST_ASSERT_EQ(runner, result.total_clauses, 3);
    }
    runner.end_test();

    runner.begin_test("Derandomized beats random on average");
    {
        std::vector<CNFClause> clauses;
        for (int i = 0; i < 15; ++i) {
            CNFClause c;
            c.literals = {i % 5 + 1, (i + 1) % 5 + 1};
            clauses.push_back(c);
        }
        auto derand = derandomized_max_sat(clauses, 5);
        // Derandomized should satisfy at least the expected value
        double expected = 0.0;
        for (const auto& c : clauses) {
            expected += 1.0 - std::pow(0.5, c.literals.size());
        }
        TEST_ASSERT_TRUE(runner, derand.clauses_satisfied >= static_cast<int>(expected));
    }
    runner.end_test();

    runner.begin_test("Discrepancy coloring");
    {
        std::vector<std::vector<int>> sets = {{0,1,2}, {1,2,3}, {0,3}};
        auto disc = derandomized_discrepancy(sets, 4);
        TEST_ASSERT_TRUE(runner, disc.max_discrepancy >= 0);
        TEST_ASSERT_EQ(runner, static_cast<int>(disc.coloring.size()), 4);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Randomized Shortest Paths
// ============================================================
void test_shortest_paths() {
    ral_test::TestRunner runner("Randomized Shortest Paths");

    runner.begin_test("Dijkstra correctness");
    {
        std::vector<WeightedEdge> edges = {{0,1,1.0},{1,2,2.0},{0,2,5.0}};
        auto result = dijkstra(3, edges, 0);
        TEST_ASSERT_NEAR(runner, result.dist[0][0], 0.0, 1e-9);
        TEST_ASSERT_NEAR(runner, result.dist[0][1], 1.0, 1e-9);
        TEST_ASSERT_NEAR(runner, result.dist[0][2], 3.0, 1e-9);
    }
    runner.end_test();

    runner.begin_test("Bellman-Ford handles negative weights");
    {
        std::vector<WeightedEdge> edges = {{0,1,2.0},{1,2,-3.0},{0,2,5.0}};
        auto result = randomized_bellman_ford(3, edges, 42);
        TEST_ASSERT_TRUE(runner, !result.has_negative_cycle);
        TEST_ASSERT_NEAR(runner, result.dist[0][2], -1.0, 1e-9);
    }
    runner.end_test();

    runner.begin_test("Negative cycle detection");
    {
        std::vector<WeightedEdge> edges = {{0,1,1.0},{1,2,-3.0},{2,0,0.5}};
        auto result = randomized_bellman_ford(3, edges, 42);
        TEST_ASSERT_TRUE(runner, result.has_negative_cycle);
    }
    runner.end_test();

    runner.begin_test("Random reweighting");
    {
        std::vector<WeightedEdge> edges = {{0,1,-2.0},{1,2,3.0}};
        auto rw = random_reweight(3, edges, 42);
        TEST_ASSERT_EQ(runner, static_cast<int>(rw.potentials.size()), 3);
        for (const auto& e : rw.reweighted_edges) {
            TEST_ASSERT_TRUE(runner, e.weight >= 0.0);
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// MAX-SAT
// ============================================================
void test_max_sat() {
    ral_test::TestRunner runner("MAX-SAT");

    runner.begin_test("Random assignment finds solution");
    {
        auto inst = random_max_sat_instance(5, 10, 3, 42);
        auto result = random_assignment(inst, 1000, 42);
        TEST_ASSERT_TRUE(runner, result.weight_satisfied > 0);
    }
    runner.end_test();

    runner.begin_test("Greedy improves over random");
    {
        auto inst = random_max_sat_instance(8, 20, 3, 42);
        auto random = random_assignment(inst, 500, 42);
        auto greedy = greedy_max_sat(inst, 50, 42);
        TEST_ASSERT_TRUE(runner, greedy.weight_satisfied >= random.weight_satisfied * 0.9);
    }
    runner.end_test();

    runner.begin_test("Randomized rounding");
    {
        auto inst = random_max_sat_instance(6, 15, 3, 42);
        auto result = randomized_rounding_max_sat(inst, 42);
        TEST_ASSERT_TRUE(runner, result.weight_satisfied > 0);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Trapezoidal Decomposition
// ============================================================
void test_trapezoidal() {
    ral_test::TestRunner runner("Trapezoidal Decomposition");

    runner.begin_test("Build and verify");
    {
        auto segs = random_segments(5, 50.0, 42);
        auto decomp = build_trapezoidal(segs);
        TEST_ASSERT_TRUE(runner, verify_decomposition(decomp));
        TEST_ASSERT_TRUE(runner, decomp.trapezoids.size() > 0);
    }
    runner.end_test();

    runner.begin_test("Point location finds a trapezoid");
    {
        auto segs = random_segments(8, 50.0, 42);
        auto decomp = build_trapezoidal(segs);
        int trap = locate_point(decomp, 25.0, 25.0);
        TEST_ASSERT_TRUE(runner, trap >= 0);
    }
    runner.end_test();

    runner.begin_test("Various segment counts");
    {
        for (int n : {1, 3, 5, 10}) {
            auto segs = random_segments(n, 100.0, n * 7);
            auto decomp = build_trapezoidal(segs);
            TEST_ASSERT_TRUE(runner, verify_decomposition(decomp));
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

int main() {
    test_stable_marriage();
    test_game_tree();
    test_dnf_counting();
    test_expander();
    test_markov_chain();
    test_conditional_expectation();
    test_shortest_paths();
    test_max_sat();
    test_trapezoidal();

    return ral_test::run_all_suites() ? 0 : 1;
}
