#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <set>
#include <string>
#include "ral.h"

// High-resolution clock helper
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time_;
public:
    Timer() { reset(); }
    void reset() { start_time_ = std::chrono::high_resolution_clock::now(); }
    double elapsed_ms() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end_time - start_time_).count();
    }
};

// Integer matrix multiplication helper
std::vector<std::vector<long long>> multiply_long_long(
    const std::vector<std::vector<long long>>& A,
    const std::vector<std::vector<long long>>& B) 
{
    int m = A.size();
    int n = A[0].size();
    int p = B[0].size();
    std::vector<std::vector<long long>> C(m, std::vector<long long>(p, 0));
    for (int i = 0; i < m; ++i) {
        for (int k = 0; k < n; ++k) {
            long long aik = A[i][k];
            for (int j = 0; j < p; ++j) {
                C[i][j] += aik * B[k][j];
            }
        }
    }
    return C;
}

// Generate random weighted graph for MST benchmarks
ral::WeightedGraph make_random_weighted_graph(int n, double p, std::mt19937& rng) {
    ral::WeightedGraph G(n);
    std::uniform_real_distribution<double> dist(1.0, 100.0);
    std::uniform_real_distribution<double> p_dist(0.0, 1.0);
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (p_dist(rng) < p) {
                G.add_edge(i, j, dist(rng));
            }
        }
    }
    return G;
}

// Generate random unweighted graph
ral::Graph make_random_graph(int n, double p, std::mt19937& rng) {
    ral::Graph G(n);
    std::uniform_real_distribution<double> p_dist(0.0, 1.0);
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (p_dist(rng) < p) {
                G.add_edge(i, j);
            }
        }
    }
    return G;
}

// ============================================================
// Benchmarks by Category
// ============================================================

void benchmark_graph_algorithms(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 1: Graph Algorithms\n";
    std::cout << "==============================================================\n";

    Timer t;

    // 1. Min-Cut: Karger vs Karger-Stein
    std::cout << "\n--- 1.1 Min-Cut Algorithms ---\n";
    std::cout << "  Vertices  Karger (10 runs)   Karger-Stein (2 runs)   Ratio\n";
    std::cout << "  ------------------------------------------------------------\n";
    for (int n : {10, 20}) {
        auto G = make_random_graph(n, 0.4, rng);
        ral::Multigraph mg(n);
        for (int u = 0; u < n; ++u) {
            for (int v : G.adj[u]) {
                if (u < v) mg.add_edge(u, v);
            }
        }

        t.reset();
        int c1 = ral::karger_repeated(mg, 10);
        double t_karger = t.elapsed_ms();

        t.reset();
        int c2 = ral::karger_stein(mg, 2);
        double t_ks = t.elapsed_ms();

        (void)c1; (void)c2;

        std::cout << "    " << std::setw(3) << n 
                  << "       " << std::setw(11) << std::fixed << std::setprecision(4) << t_karger << " ms"
                  << "       " << std::setw(15) << t_ks << " ms"
                  << "       " << std::setw(5) << std::setprecision(1) << (t_ks / (t_karger > 0.0 ? t_karger : 1e-9)) << "x\n";
    }

    // 2. MST: Kruskal vs KKT
    std::cout << "\n--- 1.2 Minimum Spanning Tree (Kruskal vs KKT) ---\n";
    std::cout << "  Vertices  Edges  Kruskal          KKT MST\n";
    std::cout << "  -------------------------------------------\n";
    for (int n : {50, 100}) {
        auto wg = make_random_weighted_graph(n, 0.3, rng);
        t.reset();
        double w1 = ral::kruskal_mst(wg);
        double t_krus = t.elapsed_ms();

        t.reset();
        double w2 = ral::kkt_mst(wg);
        double t_kkt = t.elapsed_ms();

        (void)w1; (void)w2;

        std::cout << "    " << std::setw(3) << n 
                  << "     " << std::setw(4) << wg.edges.size()
                  << "  " << std::setw(11) << std::fixed << std::setprecision(4) << t_krus << " ms"
                  << "    " << std::setw(10) << t_kkt << " ms\n";
    }

    // 3. APSP: Floyd-Warshall vs Seidel (Unweighted)
    std::cout << "\n--- 1.3 All-Pairs Shortest Path (Floyd-Warshall vs Seidel) ---\n";
    std::cout << "  Vertices  Floyd-Warshall     Seidel APSP\n";
    std::cout << "  -------------------------------------------\n";
    for (int n : {20, 50}) {
        auto G = make_random_graph(n, 0.4, rng);
        
        // Build matrices compatible with APSP algorithms
        std::vector<std::vector<double>> W(n, std::vector<double>(n, 1e9));
        std::vector<std::vector<bool>> B(n, std::vector<bool>(n, false));
        for (int i = 0; i < n; ++i) {
            W[i][i] = 0.0;
            for (int v : G.adj[i]) {
                W[i][v] = 1.0;
                B[i][v] = true;
            }
        }

        t.reset();
        auto d1 = ral::floyd_warshall(W);
        double t_fw = t.elapsed_ms();

        t.reset();
        auto d2 = ral::seidel_apsp_unweighted(B);
        double t_seidel = t.elapsed_ms();

        (void)d1; (void)d2;

        std::cout << "    " << std::setw(3) << n 
                  << "     " << std::setw(11) << std::fixed << std::setprecision(4) << t_fw << " ms"
                  << "    " << std::setw(10) << t_seidel << " ms\n";
    }
}

void benchmark_data_structures(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 2: Data Structures\n";
    std::cout << "==============================================================\n";

    Timer t;

    // 1. Dynamic Dictionaries (Treap vs SkipList vs std::set)
    std::cout << "\n--- 2.1 Search Performance (Treap vs SkipList vs std::set) ---\n";
    std::cout << "  Keys      Treap Search       SkipList Search     std::set Find\n";
    std::cout << "  --------------------------------------------------------------\n";
    for (int n : {10000, 50000}) {
        ral::Treap treap(rng);
        ral::SkipList skip_list(rng);
        std::set<int> sset;

        std::vector<int> keys(n);
        for (int i = 0; i < n; ++i) keys[i] = i;
        std::shuffle(keys.begin(), keys.end(), rng);

        for (int key : keys) {
            treap.insert(key);
            skip_list.insert(key);
            sset.insert(key);
        }

        std::vector<int> queries = keys;
        std::shuffle(queries.begin(), queries.end(), rng);

        t.reset();
        for (int q : queries) treap.search(q);
        double t_treap = t.elapsed_ms();

        t.reset();
        for (int q : queries) skip_list.search(q);
        double t_skip = t.elapsed_ms();

        t.reset();
        for (int q : queries) sset.find(q);
        double t_set = t.elapsed_ms();

        std::cout << "  " << std::setw(5) << n 
                  << "     " << std::setw(10) << std::fixed << std::setprecision(4) << t_treap << " ms"
                  << "     " << std::setw(13) << t_skip << " ms"
                  << "     " << std::setw(12) << t_set << " ms\n";
    }

    // 2. Hash Tables (Chained Universal vs FKS Perfect Hashing)
    std::cout << "\n--- 2.2 Hash Tables (Chained vs FKS Perfect Hashing) ---\n";
    std::cout << "  Keys      Chained Hash       Perfect Hash\n";
    std::cout << "  -------------------------------------------\n";
    for (int n : {1000, 5000}) {
        ral::ChainedHashTable chained(n / 2, rng);
        std::vector<int> keys(n);
        for (int i = 0; i < n; ++i) keys[i] = i * 7 + 3;

        // Build tables
        for (int key : keys) chained.insert(key);
        
        ral::PerfectHashTable perfect(n / 2, rng);
        for (int key : keys) perfect.insert(key);

        std::vector<int> queries = keys;
        std::shuffle(queries.begin(), queries.end(), rng);

        t.reset();
        for (int q : queries) chained.search(q);
        double t_chain = t.elapsed_ms();

        t.reset();
        for (int q : queries) perfect.search(q);
        double t_perf = t.elapsed_ms();

        std::cout << "  " << std::setw(4) << n 
                  << "     " << std::setw(10) << std::fixed << std::setprecision(4) << t_chain << " ms"
                  << "    " << std::setw(10) << t_perf << " ms\n";
    }
}

void benchmark_algebraic_algorithms(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 3: Algebraic & String Algorithms\n";
    std::cout << "==============================================================\n";

    Timer t;

    // 1. Freivalds Verification
    std::cout << "\n--- 3.1 Freivalds Matrix Verification (vs O(N^3) Multiply) ---\n";
    std::cout << "  N      Freivalds (O(N^2))    Full Multiply (O(N^3))\n";
    std::cout << "  -----------------------------------------------------\n";
    for (int n : {100, 200, 400}) {
        std::vector<std::vector<long long>> A(n, std::vector<long long>(n, 1));
        std::vector<std::vector<long long>> B(n, std::vector<long long>(n, 2));
        auto C = multiply_long_long(A, B);

        t.reset();
        bool v1 = ral::freivalds_verify(A, B, C, 5, rng);
        double t_frei = t.elapsed_ms();

        t.reset();
        auto AB = multiply_long_long(A, B);
        double t_mult = t.elapsed_ms();

        (void)v1; (void)AB;

        std::cout << "  " << std::setw(3) << n 
                  << "    " << std::setw(12) << std::fixed << std::setprecision(4) << t_frei << " ms"
                  << "     " << std::setw(15) << t_mult << " ms\n";
    }

    // 2. String Match (Rabin-Karp vs Naive)
    std::cout << "\n--- 3.2 String Matching (Rabin-Karp vs Naive Search) ---\n";
    std::cout << "  Text Len   Pattern Len   Rabin-Karp         Naive\n";
    std::cout << "  -----------------------------------------------------\n";
    for (int text_len : {10000, 50000}) {
        std::string text(text_len, 'A');
        std::string pattern = "AAAAAAB"; // almost matching
        text[text_len - 2] = 'B'; 

        t.reset();
        auto matches1 = ral::rabin_karp_search(text, pattern);
        double t_rk = t.elapsed_ms();

        t.reset();
        std::vector<int> matches2;
        for (int i = 0; i <= text_len - (int)pattern.length(); ++i) {
            if (text.compare(i, pattern.length(), pattern) == 0) {
                matches2.push_back(i);
            }
        }
        double t_naive = t.elapsed_ms();

        (void)matches1; (void)matches2;

        std::cout << "   " << std::setw(5) << text_len 
                  << "        " << std::setw(3) << pattern.length()
                  << "      " << std::setw(10) << std::fixed << std::setprecision(4) << t_rk << " ms"
                  << "    " << std::setw(10) << t_naive << " ms\n";
    }
}

void benchmark_geometry_algorithms(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 4: Geometric & Linear Programming\n";
    std::cout << "==============================================================\n";

    Timer t;

    // 1. Convex Hull (RIC vs Jarvis)
    std::cout << "\n--- 4.1 Convex Hull (RIC vs Jarvis March) ---\n";
    std::cout << "  Points    Convex Hull RIC    Jarvis March\n";
    std::cout << "  -------------------------------------------\n";
    for (int n : {500, 2000}) {
        std::vector<ral::Point2D> pts(n);
        std::uniform_real_distribution<double> coords(-100.0, 100.0);
        for (int i = 0; i < n; ++i) {
            pts[i] = {coords(rng), coords(rng)};
        }

        t.reset();
        auto hull1 = ral::convex_hull(pts);
        double t_ric = t.elapsed_ms();

        t.reset();
        auto hull2 = ral::gift_wrap_hull(pts);
        double t_jarvis = t.elapsed_ms();

        (void)hull1; (void)hull2;

        std::cout << "   " << std::setw(4) << n 
                  << "     " << std::setw(11) << std::fixed << std::setprecision(4) << t_ric << " ms"
                  << "    " << std::setw(10) << t_jarvis << " ms\n";
    }

    // 2. Linear Programming (Seidel LP Solver)
    std::cout << "\n--- 4.2 Seidel's Randomized LP Solver ---\n";
    std::cout << "  Constraints    Variables    LP Run Time\n";
    std::cout << "  -------------------------------------------\n";
    for (int count : {100, 500}) {
        // Build matrices compatible with Seidel's LP solver
        std::vector<std::vector<double>> A(count, std::vector<double>(2));
        std::vector<double> b(count);
        std::uniform_real_distribution<double> coeffs(-10.0, 10.0);
        for (int i = 0; i < count; ++i) {
            A[i][0] = coeffs(rng);
            A[i][1] = coeffs(rng);
            b[i] = std::abs(coeffs(rng));
        }
        std::vector<double> c = {-2.0, -1.0}; // Objective

        t.reset();
        auto res = ral::solve_lp(A, b, c, 2);
        double t_lp = t.elapsed_ms();

        (void)res;

        std::cout << "      " << std::setw(3) << count 
                  << "            2       " 
                  << std::setw(10) << std::fixed << std::setprecision(4) << t_lp << " ms\n";
    }
}

void benchmark_probability_selection(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 5: Selection & Online Algorithms\n";
    std::cout << "==============================================================\n";

    Timer t;

    // 1. Selection (QuickSelect vs LazySelect vs std::nth_element)
    std::cout << "\n--- 5.1 Selection (QuickSelect vs LazySelect vs nth_element) ---\n";
    std::cout << "  N         QuickSelect       LazySelect        std::nth_element\n";
    std::cout << "  --------------------------------------------------------------\n";
    for (int n : {10000, 50000}) {
        std::vector<int> S(n);
        for (int i = 0; i < n; ++i) S[i] = i;
        std::shuffle(S.begin(), S.end(), rng);
        int k = n / 2;

        std::vector<int> copy1 = S;
        t.reset();
        int v1 = ral::randomized_find(copy1, k);
        double t_qs = t.elapsed_ms();

        std::vector<int> copy2 = S;
        t.reset();
        auto v2 = ral::lazy_select(copy2, k, rng);
        double t_ls = t.elapsed_ms();

        std::vector<int> copy3 = S;
        t.reset();
        std::nth_element(copy3.begin(), copy3.begin() + k - 1, copy3.end());
        int v3 = copy3[k - 1];
        double t_nth = t.elapsed_ms();

        (void)v1; (void)v2; (void)v3;

        std::cout << "  " << std::setw(5) << n 
                  << "    " << std::setw(10) << std::fixed << std::setprecision(4) << t_qs << " ms"
                  << "    " << std::setw(10) << t_ls << " ms"
                  << "    " << std::setw(12) << t_nth << " ms\n";
    }

    // 2. Online Paging (LRU vs Marking Paging)
    std::cout << "\n--- 5.2 Paging Miss Rate Simulation ---\n";
    std::cout << "  Requests    Cache Size    LRU Misses    Marking Misses\n";
    std::cout << "  -------------------------------------------------------\n";
    for (int count : {1000, 5000}) {
        std::vector<int> reqs(count);
        std::uniform_int_distribution<int> page_dist(1, 20);
        for (int i = 0; i < count; ++i) reqs[i] = page_dist(rng);

        int lru_m = ral::deterministic_lru(5, reqs);
        int mark_m = ral::marking_paging(5, reqs);

        std::cout << "    " << std::setw(4) << count 
                  << "           5        " 
                  << std::setw(6) << lru_m 
                  << "        " << std::setw(6) << mark_m << "\n";
    }
}

void benchmark_modern_algorithms(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 6: Modern Sketching & Dimension Projections\n";
    std::cout << "==============================================================\n";

    Timer t;

    // 1. Cardinality: HLL vs std::unordered_set
    std::cout << "\n--- 6.1 Unique Count (HyperLogLog vs std::unordered_set) ---\n";
    std::cout << "  Items     HLL Time        HLL Memory      std::set Time     std::set Memory\n";
    std::cout << "  ----------------------------------------------------------------------------\n";
    for (int n : {50000, 200000}) {
        t.reset();
        ral::HyperLogLog hll(12);
        for (int i = 0; i < n; ++i) hll.insert(i);
        double t_hll = t.elapsed_ms();
        double m_hll = 4.0; // 4 KB

        t.reset();
        std::unordered_set<int> uset;
        uset.reserve(n);
        for (int i = 0; i < n; ++i) uset.insert(i);
        double t_set = t.elapsed_ms();
        double m_set = (static_cast<double>(n) * 32.0) / (1024.0 * 1024.0); // in MB

        std::cout << "  " << std::setw(6) << n 
                  << "   " << std::setw(9) << std::fixed << std::setprecision(2) << t_hll << " ms"
                  << "       " << std::setw(6) << std::setprecision(1) << m_hll << " KB"
                  << "      " << std::setw(9) << std::setprecision(2) << t_set << " ms"
                  << "      " << std::setw(8) << std::setprecision(1) << m_set << " MB\n";
    }

    // 2. Random Projections (Johnson-Lindenstrauss)
    std::cout << "\n--- 6.2 Johnson-Lindenstrauss Random Projection ---\n";
    std::cout << "  Original Dim    Target Dim    Projection Time\n";
    std::cout << "  -----------------------------------------------\n";
    for (int dim : {1000, 5000}) {
        std::vector<double> point(dim, 1.0);
        ral::RandomProjection rp(dim, 100, rng);

        t.reset();
        auto res = rp.project(point);
        double t_rp = t.elapsed_ms();

        (void)res;

        std::cout << "      " << std::setw(4) << dim 
                  << "           100        "
                  << std::fixed << std::setprecision(4) << t_rp << " ms\n";
    }
}

void benchmark_reservoir_sampling(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 7: Reservoir Sampling\n";
    std::cout << "==============================================================\n";

    Timer t;
    int k = 100;

    std::cout << "\n--- 7.1 Reservoir Sampling (k=" << k << ") ---\n";
    std::cout << "  Stream Size   Basic Reservoir   Weighted Reservoir   std::sample\n";
    std::cout << "  -------------------------------------------------------------------\n";
    for (int n : {1000, 10000, 100000}) {
        std::vector<int> stream(n);
        for (int i = 0; i < n; ++i) stream[i] = i;

        std::vector<double> weights(n, 1.0);
        std::uniform_real_distribution<double> wdist(0.1, 10.0);
        for (int i = 0; i < n; ++i) weights[i] = wdist(rng);

        t.reset();
        auto s1 = ral::reservoir_sample(stream, k, rng);
        double t_basic = t.elapsed_ms();

        t.reset();
        auto s2 = ral::weighted_reservoir_sample(stream, weights, k, rng);
        double t_weighted = t.elapsed_ms();

        std::vector<int> out(k);
        t.reset();
        std::sample(stream.begin(), stream.end(), out.begin(), k, rng);
        double t_std = t.elapsed_ms();

        (void)s1; (void)s2;

        std::cout << "  " << std::setw(7) << n
                  << "     " << std::setw(10) << std::fixed << std::setprecision(4) << t_basic << " ms"
                  << "       " << std::setw(10) << t_weighted << " ms"
                  << "      " << std::setw(10) << t_std << " ms\n";
    }
}

void benchmark_pollard_rho(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 8: Pollard's Rho Factorization\n";
    std::cout << "==============================================================\n";

    Timer t;

    std::cout << "\n--- 8.1 Factorization of Increasing-Size Integers ---\n";
    std::cout << "  Number               Type          Time (ms)   Factors\n";
    std::cout << "  -----------------------------------------------------------------\n";

    std::vector<std::pair<long long, std::string>> numbers = {
        {1000003LL, "prime"},
        {1000000007LL, "prime"},
        {123456789012345LL, "composite"},
        {600851475143LL, "composite"}
    };

    for (auto& [n, label] : numbers) {
        t.reset();
        auto factors = ral::pollard_rho_factorize(n, rng);
        double elapsed = t.elapsed_ms();

        std::cout << "  " << std::setw(16) << n
                  << "   " << std::setw(11) << std::left << label << std::right
                  << "   " << std::setw(8) << std::fixed << std::setprecision(3) << elapsed
                  << "     ";
        for (size_t i = 0; i < factors.size(); ++i) {
            if (i > 0) std::cout << " * ";
            std::cout << factors[i];
        }
        std::cout << "\n";
    }
}

void benchmark_welzl_mec(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 9: Minimum Enclosing Circle\n";
    std::cout << "==============================================================\n";

    Timer t;

    std::cout << "\n--- 9.1 Welzl MEC vs Brute-Force O(n^4) ---\n";
    std::cout << "  Points   Welzl (ms)   Brute-Force (ms)   Radius Match\n";
    std::cout << "  ---------------------------------------------------------\n";

    for (int n : {10, 20, 50, 100, 500}) {
        std::uniform_real_distribution<double> dist(-100.0, 100.0);
        std::vector<ral::Point2D_mec> pts(n);
        for (auto& p : pts) p = {dist(rng), dist(rng)};

        t.reset();
        auto c_welzl = ral::welzl_mec(pts, rng);
        double t_welzl = t.elapsed_ms();

        std::cout << "  " << std::setw(5) << n
                  << "   " << std::setw(9) << std::fixed << std::setprecision(4) << t_welzl;

        if (n <= 50) {
            t.reset();
            double min_r = 1e18;
            for (int i = 0; i < n; ++i) {
                for (int j = i + 1; j < n; ++j) {
                    auto c2 = ral::circle_from_2(pts[i], pts[j]);
                    bool covers = true;
                    for (auto& p : pts) { covers &= c2.contains(p); if (!covers) break; }
                    if (covers && c2.radius < min_r) min_r = c2.radius;
                }
                for (int j = i + 1; j < n; ++j) {
                    for (int k = j + 1; k < n; ++k) {
                        auto c3 = ral::circle_from_3(pts[i], pts[j], pts[k]);
                        bool covers = true;
                        for (auto& p : pts) { covers &= c3.contains(p); if (!covers) break; }
                        if (covers && c3.radius < min_r) min_r = c3.radius;
                    }
                }
            }
            double t_bf = t.elapsed_ms();
            bool match = std::abs(c_welzl.radius - min_r) < 1e-6;
            std::cout << "   " << std::setw(11) << t_bf
                      << "         " << (match ? "YES" : "NO") << "\n";
        } else {
            std::cout << "       (skipped)           ---\n";
        }
    }
}

void benchmark_streaming(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 10: Streaming Algorithms\n";
    std::cout << "==============================================================\n";

    Timer t;

    std::cout << "\n--- 10.1 Streaming Sketch Performance & Memory ---\n";
    std::cout << "  Stream Size   CMS Time    FM Time    MG Time    Est. Memory\n";
    std::cout << "  ----------------------------------------------------------------\n";

    for (int n : {10000, 100000, 1000000}) {
        std::uniform_int_distribution<int> val_dist(1, 1000);

        t.reset();
        ral::StreamingCountMinSketch cms(0.001, 0.01, rng);
        for (int i = 0; i < n; ++i) {
            cms.update("item_" + std::to_string(val_dist(rng)));
        }
        double t_cms = t.elapsed_ms();
        double mem_cms = static_cast<double>(cms.depth()) * cms.width() * sizeof(long long) / 1024.0;

        t.reset();
        ral::FlajoletMartin fm(64, rng);
        for (int i = 0; i < n; ++i) {
            fm.update("item_" + std::to_string(val_dist(rng)));
        }
        double t_fm = t.elapsed_ms();
        double mem_fm = 64.0 * sizeof(uint64_t) / 1024.0 + 64.0 * sizeof(uint64_t) / 1024.0;

        t.reset();
        ral::MisraGries mg(10);
        for (int i = 0; i < n; ++i) {
            mg.update("item_" + std::to_string(val_dist(rng)));
        }
        double t_mg = t.elapsed_ms();
        double mem_mg = 10.0 * (64.0 + sizeof(long long)) / 1024.0;

        std::cout << "  " << std::setw(8) << n
                  << "    " << std::setw(6) << std::fixed << std::setprecision(3) << t_cms << " ms"
                  << "   " << std::setw(6) << t_fm << " ms"
                  << "   " << std::setw(6) << t_mg << " ms"
                  << "    CMS:" << std::setprecision(1) << mem_cms << "KB"
                  << " FM:" << mem_fm << "KB"
                  << " MG:" << mem_mg << "KB\n";
    }
}

void benchmark_lsh(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 11: Locality-Sensitive Hashing\n";
    std::cout << "==============================================================\n";

    Timer t;

    int dim = 50;
    int n = 1000;
    int num_queries = 100;
    int k_neighbors = 10;
    int num_hashes = 16;

    std::cout << "\n--- 11.1 LSH vs Brute-Force Nearest Neighbor ---\n";
    std::cout << "  Vectors   Dim   LSH Time     BF Time      Recall@" << k_neighbors << "\n";
    std::cout << "  -----------------------------------------------------------\n";

    std::normal_distribution<double> norm(0.0, 1.0);
    std::vector<std::vector<double>> data(n, std::vector<double>(dim));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < dim; ++j) data[i][j] = norm(rng);
    }

    ral::LSHIndex index(dim, num_hashes, rng);
    for (int i = 0; i < n; ++i) index.insert(data[i], i);

    std::vector<std::vector<double>> queries(num_queries, std::vector<double>(dim));
    for (int i = 0; i < num_queries; ++i) {
        for (int j = 0; j < dim; ++j) queries[i][j] = norm(rng);
    }

    t.reset();
    std::vector<std::vector<ral::LSHResult>> lsh_results(num_queries);
    for (int i = 0; i < num_queries; ++i) {
        lsh_results[i] = index.query(queries[i], k_neighbors);
    }
    double t_lsh = t.elapsed_ms();

    t.reset();
    std::vector<std::vector<ral::LSHResult>> bf_results(num_queries);
    for (int i = 0; i < num_queries; ++i) {
        bf_results[i] = ral::brute_force_nn(data, queries[i], k_neighbors);
    }
    double t_bf = t.elapsed_ms();

    int total_recall = 0;
    for (int i = 0; i < num_queries; ++i) {
        for (auto& lr : lsh_results[i]) {
            for (auto& br : bf_results[i]) {
                if (lr.index == br.index) { total_recall++; break; }
            }
        }
    }
    double recall = 100.0 * total_recall / (num_queries * k_neighbors);

    std::cout << "  " << std::setw(5) << n
              << "   " << std::setw(3) << dim
              << "   " << std::setw(7) << std::fixed << std::setprecision(3) << t_lsh << " ms"
              << "   " << std::setw(8) << t_bf << " ms"
              << "     " << std::setprecision(1) << recall << "%\n";
}

void benchmark_differential_privacy(std::mt19937& rng) {
    std::cout << "\n==============================================================\n";
    std::cout << "  CATEGORY 12: Differential Privacy\n";
    std::cout << "  (epsilon = 1.0)\n";
    std::cout << "==============================================================\n";

    Timer t;
    double eps = 1.0;

    std::cout << "\n--- 12.1 Private Mean (Laplace Mechanism) ---\n";
    std::cout << "  Dataset Size   True Mean     Private Mean   Abs Error    Time (ms)\n";
    std::cout << "  -------------------------------------------------------------------\n";

    for (int n : {1000, 10000, 100000}) {
        std::normal_distribution<double> dist(50000.0, 15000.0);
        std::vector<double> data(n);
        double true_sum = 0.0;
        for (int i = 0; i < n; ++i) {
            data[i] = std::max(10000.0, dist(rng));
            true_sum += data[i];
        }
        double true_mean = true_sum / n;

        t.reset();
        double priv_mean = ral::private_mean(data, 100000.0, eps, rng);
        double elapsed = t.elapsed_ms();

        std::cout << "  " << std::setw(8) << n
                  << "    " << std::setw(10) << std::fixed << std::setprecision(2) << true_mean
                  << "     " << std::setw(10) << priv_mean
                  << "   " << std::setw(8) << std::setprecision(2) << std::abs(priv_mean - true_mean)
                  << "   " << std::setw(8) << std::setprecision(4) << elapsed << "\n";
    }

    std::cout << "\n--- 12.2 Private Histogram (Laplace Mechanism) ---\n";
    std::cout << "  Dataset Size   Buckets   Time (ms)\n";
    std::cout << "  -----------------------------------\n";

    for (int n : {1000, 10000, 100000}) {
        int buckets = 10;
        std::uniform_int_distribution<int> bucket_dist(0, buckets - 1);
        std::vector<int> data(n);
        for (int i = 0; i < n; ++i) data[i] = bucket_dist(rng);

        t.reset();
        auto hist = ral::private_histogram(data, buckets, eps, rng);
        double elapsed = t.elapsed_ms();

        std::cout << "  " << std::setw(8) << n
                  << "       " << std::setw(2) << buckets
                  << "     " << std::setw(8) << std::fixed << std::setprecision(4) << elapsed << "\n";
    }
}

int main() {
    std::cout << "==============================================================\n";
    std::cout << "          RAL Full Randomized Algorithms Benchmark Suite\n";
    std::cout << "==============================================================\n";

    std::mt19937 rng(54321);

    benchmark_graph_algorithms(rng);
    benchmark_data_structures(rng);
    benchmark_algebraic_algorithms(rng);
    benchmark_geometry_algorithms(rng);
    benchmark_probability_selection(rng);
    benchmark_modern_algorithms(rng);
    benchmark_reservoir_sampling(rng);
    benchmark_pollard_rho(rng);
    benchmark_welzl_mec(rng);
    benchmark_streaming(rng);
    benchmark_lsh(rng);
    benchmark_differential_privacy(rng);

    std::cout << "\n==============================================================\n";
    return 0;
}
