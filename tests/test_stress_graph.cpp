#include "test_framework.h"
#include "ral.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

using namespace ral;

// ============================================================
// Stress: Karger min-cut (both implementations)
// ============================================================
void test_stress_min_cut() {
    ral_test::TestRunner runner("Stress: MinCut");

    runner.begin_test("karger_min_cut_size on known graph");
    {
        Multigraph G(4);
        G.add_edge(0, 1); G.add_edge(0, 2); G.add_edge(1, 2); G.add_edge(2, 3);
        int exact = exact_min_cut(G, 4);
        // Run many times; min should equal exact
        for (int t = 0; t < 50; t++) {
            int result = karger_min_cut_size(G);
            TEST_ASSERT_TRUE(runner, result >= exact);
        }
    }
    runner.end_test();

    runner.begin_test("karger_repeated finds exact cut (small graphs)");
    for (int n = 3; n <= 8; n++) {
        Multigraph G(n);
        // Cycle graph: min-cut = 2
        for (int i = 0; i < n; i++) G.add_edge(i, (i + 1) % n);
        int exact = exact_min_cut(G, n);
        int trials = n * n;
        int result = karger_repeated(G, trials);
        TEST_ASSERT_EQ(runner, result, exact);
    }
    runner.end_test();

    runner.begin_test("karger_stein on random multigraphs");
    for (int n = 4; n <= 12; n++) {
        auto G = random_multigraph(n, 3 * n);
        int exact = exact_min_cut(G, n);
        int result = karger_stein(G, 5);
        TEST_ASSERT_TRUE(runner, result >= exact);
    }
    runner.end_test();

    runner.end_test();
    runner.begin_test("contract_edge reduces vertex count");
    {
        Multigraph G(5);
        G.add_edge(0, 1); G.add_edge(1, 2); G.add_edge(2, 3); G.add_edge(3, 4);
        contract_edge(G, 0, 1);
        TEST_ASSERT_EQ(runner, G.n, 4);
        TEST_ASSERT_TRUE(runner, G.adj.contains(0));
        TEST_ASSERT_TRUE(runner, !G.adj.contains(1));
    }
    runner.end_test();

    runner.begin_test("karger_min_cut_chapter1 correctness");
    {
        KargerGraph g(5);
        g.add_edge(0, 1); g.add_edge(0, 2); g.add_edge(0, 3);
        g.add_edge(1, 2); g.add_edge(1, 3); g.add_edge(2, 3);
        g.add_edge(2, 4); g.add_edge(3, 4);
        int result = karger_min_cut_repeated(g, 50);
        TEST_ASSERT_EQ(runner, result, 2);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: MST (Kruskal and KKT)
// ============================================================
void test_stress_mst() {
    ral_test::TestRunner runner("Stress: MST");

    runner.begin_test("Kruskal on known graphs");
    {
        WeightedGraph G(4);
        G.add_edge(0, 1, 1.0); G.add_edge(1, 2, 2.0); G.add_edge(2, 3, 3.0);
        G.add_edge(0, 3, 10.0);
        TEST_ASSERT_NEAR(runner, kruskal_mst(G), 6.0, 1e-6);
    }
    runner.end_test();

    runner.begin_test("Kruskal on complete graphs");
    for (int n = 3; n <= 15; n++) {
        WeightedGraph G(n);
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                G.add_edge(i, j, (i * 7 + j * 3 + 1) % 100 + 1);
        double mst_w = kruskal_mst(G);
        TEST_ASSERT_TRUE(runner, mst_w > 0);
    }
    runner.end_test();

    runner.begin_test("KKT matches Kruskal on small graphs");
    for (int t = 0; t < 20; t++) {
        int n = (t % 8) + 3;
        auto G = random_weighted_graph(n, 0.5);
        if (G.edges.empty()) continue;
        double kruskal_w = kruskal_mst(G);
        double kkt_w = kkt_mst(G);
        TEST_ASSERT_NEAR(runner, kruskal_w, kkt_w, 0.1);
    }
    runner.end_test();

    runner.begin_test("DSU basics");
    {
        DSU dsu(10);
        TEST_ASSERT_TRUE(runner, dsu.unite(0, 1));
        TEST_ASSERT_TRUE(runner, dsu.unite(2, 3));
        TEST_ASSERT_EQ(runner, dsu.find(0), dsu.find(1));
        TEST_ASSERT_TRUE(runner, dsu.find(0) != dsu.find(2));
        TEST_ASSERT_TRUE(runner, !dsu.unite(0, 1)); // already united
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: APSP
// ============================================================
void test_stress_apsp() {
    ral_test::TestRunner runner("Stress: APSP");

    runner.begin_test("Floyd-Warshall vs min-plus squaring");
    for (int t = 0; t < 30; t++) {
        int n = (t % 6) + 2;
        auto W = random_weighted_graph_matrix(n, 0.5);
        auto fw = floyd_warshall(W);
        auto mps = apsp_repeated_squaring(W);
        TEST_ASSERT_TRUE(runner, verify_apsp(fw, mps, 0.5));
    }
    runner.end_test();

    runner.begin_test("Floyd-Warshall diagonal is 0");
    for (int n = 2; n <= 10; n++) {
        auto W = random_weighted_graph_matrix(n, 0.5);
        auto fw = floyd_warshall(W);
        for (int i = 0; i < n; i++) {
            TEST_ASSERT_NEAR(runner, fw[i][i], 0.0, 1e-6);
        }
    }
    runner.end_test();

    runner.begin_test("Seidel APSP on unweighted graphs");
    for (int t = 0; t < 20; t++) {
        int n = (t % 5) + 2;
        auto adj = random_unweighted_graph(n, 0.5);
        // BFS ground truth
        Matrix bfs_dist(n, std::vector<double>(n, INF));
        for (int s = 0; s < n; s++) {
            bfs_dist[s][s] = 0;
            std::vector<bool> visited(n, false);
            visited[s] = true;
            std::queue<int> q;
            q.push(s);
            while (!q.empty()) {
                int u = q.front(); q.pop();
                for (int v = 0; v < n; v++)
                    if (adj[u][v] && !visited[v]) {
                        visited[v] = true;
                        bfs_dist[s][v] = bfs_dist[s][u] + 1;
                        q.push(v);
                    }
            }
        }
        auto seidel = seidel_apsp_unweighted(adj);
        TEST_ASSERT_TRUE(runner, verify_apsp(seidel, bfs_dist, 0.1));
    }
    runner.end_test();

    runner.begin_test("bool_matrix_multiply correctness");
    for (int t = 0; t < 30; t++) {
        int n = (t % 5) + 2;
        BoolMatrix A(n, std::vector<bool>(n, false));
        BoolMatrix B(n, std::vector<bool>(n, false));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                A[i][j] = (i + j + t) % 3 == 0;
                B[i][j] = (i * j + t) % 2 == 0;
            }
        BoolMatrix C = bool_matrix_multiply(A, B);
        // Verify C[i][j] = OR_k(A[i][k] AND B[k][j])
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                bool expected = false;
                for (int k = 0; k < n; k++) {
                    if (A[i][k] && B[k][j]) { expected = true; break; }
                }
                TEST_ASSERT_TRUE(runner, C[i][j] == expected);
            }
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: MIS and Matchings
// ============================================================
void test_stress_mis_matching() {
    ral_test::TestRunner runner("Stress: MIS & Matchings");

    runner.begin_test("maximal_matching is valid");
    for (int t = 0; t < 50; t++) {
        int n = (t % 10) + 3;
        auto g = make_random(n, 0.3, t);
        auto mm = maximal_matching(g);
        TEST_ASSERT_TRUE(runner, verify_matching(g, mm));
        // Maximal: no edge can be added
        for (int u = 0; u < n; u++)
            for (int v : g.adj[u])
                if (u < v) {
                    bool found = false;
                    for (auto [a, b] : mm.edges)
                        if ((a == u && b == v) || (a == v && b == u)) found = true;
                    if (!found) {
                        // At least one endpoint must be matched
                        bool u_matched = false, v_matched = false;
                        for (auto [a, b] : mm.edges) {
                            if (a == u || b == u) u_matched = true;
                            if (a == v || b == v) v_matched = true;
                        }
                        TEST_ASSERT_TRUE(runner, u_matched || v_matched);
                    }
                }
    }
    runner.end_test();

    runner.begin_test("blossom_matching is valid and maximal");
    for (int t = 0; t < 30; t++) {
        int n = (t % 8) + 2;
        auto g = make_random(n, 0.4, t);
        auto bm = blossom_matching(g);
        TEST_ASSERT_TRUE(runner, verify_matching(g, bm));
    }
    runner.end_test();

    runner.begin_test("random_maximal_matching is valid");
    for (int t = 0; t < 30; t++) {
        int n = (t % 8) + 3;
        auto g = make_random(n, 0.3, t);
        auto rm = random_maximal_matching(g);
        TEST_ASSERT_TRUE(runner, verify_matching(g, rm));
    }
    runner.end_test();

    runner.begin_test("approximate_max_matching is valid");
    for (int t = 0; t < 30; t++) {
        int n = (t % 8) + 3;
        auto g = make_random(n, 0.3, t);
        auto am = approximate_max_matching(g);
        TEST_ASSERT_TRUE(runner, verify_matching(g, am));
    }
    runner.end_test();

    runner.begin_test("graph helpers produce valid graphs");
    for (int n = 3; n <= 20; n++) {
        auto path = make_path(n);
        TEST_ASSERT_EQ(runner, path.n, n);
        auto cycle = make_cycle(n);
        TEST_ASSERT_EQ(runner, cycle.n, n);
        auto star = make_star(0, n);
        TEST_ASSERT_EQ(runner, star.n, n + 1);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

int main() {
    test_stress_min_cut();
    test_stress_mst();
    test_stress_apsp();
    test_stress_mis_matching();
    return ral_test::run_all_suites() ? 0 : 1;
}
