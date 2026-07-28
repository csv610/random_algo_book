#include "test_framework.h"
#include "ral.h"
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <cmath>

using namespace ral;

// ============================================================
// Stress: Treap
// ============================================================
void test_stress_treap() {
    ral_test::TestRunner runner("Stress: Treap");

    runner.begin_test("insert and search 1000 elements");
    {
        std::mt19937 rng(42);
        Treap treap(rng);
        for (int i = 1; i <= 1000; i++) treap.insert(i);
        TEST_ASSERT_EQ(runner, treap.size(), 1000);
        for (int i = 1; i <= 1000; i++) {
            TEST_ASSERT_TRUE(runner, treap.search(i));
        }
        TEST_ASSERT_TRUE(runner, !treap.search(0));
        TEST_ASSERT_TRUE(runner, !treap.search(1001));
    }
    runner.end_test();

    runner.begin_test("BST and heap properties maintained after operations");
    {
        std::mt19937 rng(123);
        Treap treap(rng);
        for (int i = 1; i <= 500; i++) treap.insert(i);
        TEST_ASSERT_TRUE(runner, treap.verify_bst());
        TEST_ASSERT_TRUE(runner, treap.verify_heap());
        for (int i = 1; i <= 250; i++) treap.remove(i);
        TEST_ASSERT_TRUE(runner, treap.verify_bst());
        TEST_ASSERT_TRUE(runner, treap.verify_heap());
        TEST_ASSERT_EQ(runner, treap.size(), 250);
    }
    runner.end_test();

    runner.begin_test("inorder is sorted");
    {
        std::mt19937 rng(99);
        Treap treap(rng);
        std::vector<int> vals = {50, 25, 75, 10, 30, 60, 90, 5, 15, 27, 35, 55, 65, 80, 95};
        for (int v : vals) treap.insert(v);
        auto inorder = treap.inorder();
        TEST_ASSERT_TRUE(runner, std::is_sorted(inorder.begin(), inorder.end()));
        TEST_ASSERT_EQ(runner, (int)inorder.size(), (int)vals.size());
    }
    runner.end_test();

    runner.begin_test("remove non-existent is safe");
    {
        std::mt19937 rng(77);
        Treap treap(rng);
        for (int i = 1; i <= 100; i++) treap.insert(i);
        treap.remove(500);
        treap.remove(0);
        TEST_ASSERT_EQ(runner, treap.size(), 100);
        TEST_ASSERT_TRUE(runner, treap.verify_bst());
    }
    runner.end_test();

    runner.begin_test("height analysis");
    {
        std::mt19937 rng(42);
        Treap treap(rng);
        int n = 10000;
        for (int i = 0; i < n; i++) treap.insert(i + 1);
        int h = treap.height();
        // Expected ~ O(log n), allow up to 5*2*log2(n)
        double bound = 5.0 * 2.0 * std::log2(n);
        TEST_ASSERT_TRUE(runner, h <= (int)bound + 1);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Skip List
// ============================================================
void test_stress_skip_list() {
    ral_test::TestRunner runner("Stress: SkipList");

    runner.begin_test("insert and search 1000 elements");
    {
        std::mt19937 rng(42);
        SkipList sl(rng);
        for (int i = 1; i <= 1000; i++) sl.insert(i);
        TEST_ASSERT_EQ(runner, sl.size(), 1000);
        for (int i = 1; i <= 1000; i++) {
            TEST_ASSERT_TRUE(runner, sl.search(i));
        }
        TEST_ASSERT_TRUE(runner, !sl.search(0));
        TEST_ASSERT_TRUE(runner, !sl.search(1001));
    }
    runner.end_test();

    runner.begin_test("remove maintains correctness");
    {
        std::mt19937 rng(99);
        SkipList sl(rng);
        for (int i = 1; i <= 500; i++) sl.insert(i);
        for (int i = 1; i <= 250; i++) sl.remove(i);
        TEST_ASSERT_EQ(runner, sl.size(), 250);
        for (int i = 1; i <= 250; i++) {
            TEST_ASSERT_TRUE(runner, !sl.search(i));
        }
        for (int i = 251; i <= 500; i++) {
            TEST_ASSERT_TRUE(runner, sl.search(i));
        }
    }
    runner.end_test();

    runner.begin_test("duplicate insert is no-op");
    {
        std::mt19937 rng(42);
        SkipList sl(rng);
        for (int i = 0; i < 100; i++) sl.insert(42);
        TEST_ASSERT_EQ(runner, sl.size(), 1);
    }
    runner.end_test();

    runner.begin_test("remove non-existent is safe");
    {
        std::mt19937 rng(42);
        SkipList sl(rng);
        for (int i = 1; i <= 100; i++) sl.insert(i * 2);
        sl.remove(999);
        sl.remove(1);
        TEST_ASSERT_EQ(runner, sl.size(), 100);
    }
    runner.end_test();

    runner.begin_test("level analysis");
    {
        std::mt19937 rng(42);
        SkipList sl(rng);
        int n = 10000;
        for (int i = 0; i < n; i++) sl.insert(i + 1);
        // Max level should be O(log n)
        double bound = 2.0 * std::log2(n);
        TEST_ASSERT_TRUE(runner, sl.max_level() <= (int)bound + 2);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Hash Tables
// ============================================================
void test_stress_hash_table() {
    ral_test::TestRunner runner("Stress: HashTables");

    runner.begin_test("UniversalHashTable insert/search/remove");
    {
        std::mt19937 rng(42);
        UniversalHashTable ht(1000, rng);
        for (int i = 1; i <= 500; i++) ht.insert(i);
        TEST_ASSERT_EQ(runner, ht.size(), 500);
        for (int i = 1; i <= 500; i++) {
            TEST_ASSERT_TRUE(runner, ht.search(i));
        }
        TEST_ASSERT_TRUE(runner, !ht.search(0));
        TEST_ASSERT_TRUE(runner, !ht.search(501));
        for (int i = 1; i <= 250; i++) ht.remove(i);
        TEST_ASSERT_EQ(runner, ht.size(), 250);
        for (int i = 1; i <= 250; i++) {
            TEST_ASSERT_TRUE(runner, !ht.search(i));
        }
        for (int i = 251; i <= 500; i++) {
            TEST_ASSERT_TRUE(runner, ht.search(i));
        }
    }
    runner.end_test();

    runner.begin_test("load factor accuracy");
    {
        std::mt19937 rng(42);
        UniversalHashTable ht(100, rng);
        for (int i = 0; i < 50; i++) ht.insert(i);
        TEST_ASSERT_NEAR(runner, ht.load_factor(), 0.5, 0.01);
    }
    runner.end_test();

    runner.begin_test("PerfectHashTable insert and search");
    {
        std::mt19937 rng(42);
        PerfectHashTable pht(100, rng);
        for (int i = 1; i <= 50; i++) pht.insert(i * 3);
        for (int i = 1; i <= 50; i++) {
            TEST_ASSERT_TRUE(runner, pht.search(i * 3));
        }
        TEST_ASSERT_TRUE(runner, !pht.search(1));
        TEST_ASSERT_TRUE(runner, !pht.search(999));
    }
    runner.end_test();

    runner.begin_test("collision analysis: avg chain <= 2 for load=1");
    {
        double total_avg = 0;
        int trials = 100;
        for (int t = 0; t < trials; t++) {
            std::mt19937 rng(t * 77777 + 1000);
            UniversalHashTable ht(100, rng);
            for (int i = 1; i <= 100; i++) ht.insert(i);
            total_avg += ht.avg_chain_length();
        }
        double avg = total_avg / trials;
        // For load factor 1, avg chain ~ 1.0
        TEST_ASSERT_TRUE(runner, avg <= 2.0);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Sketching (MinHash, CountMinSketch, HyperLogLog)
// ============================================================
void test_stress_sketching() {
    ral_test::TestRunner runner("Stress: Sketching");

    runner.begin_test("MinHash Jaccard similarity");
    {
        std::mt19937 rng(42);
        MinHash mh(200, rng);

        std::unordered_set<int> A = {1, 2, 3, 4, 5, 6, 7, 8};
        std::unordered_set<int> B = {5, 6, 7, 8, 9, 10, 11, 12};
        // True Jaccard = |A ∩ B| / |A ∪ B| = 4/12 = 1/3
        auto sigA = mh.compute_signature(A);
        auto sigB = mh.compute_signature(B);
        double est = MinHash::estimate_similarity(sigA, sigB);
        TEST_ASSERT_NEAR(runner, est, 1.0 / 3.0, 0.15);
    }
    runner.end_test();

    runner.begin_test("MinHash identical sets");
    {
        std::mt19937 rng(42);
        MinHash mh(200, rng);
        std::unordered_set<int> A = {1, 2, 3, 4, 5};
        auto sigA = mh.compute_signature(A);
        auto sigB = mh.compute_signature(A);
        double est = MinHash::estimate_similarity(sigA, sigB);
        TEST_ASSERT_NEAR(runner, est, 1.0, 0.01);
    }
    runner.end_test();

    runner.begin_test("CountMinSketch overestimate property");
    {
        std::mt19937 rng(42);
        CountMinSketch cms(100, 10, rng);
        for (int i = 0; i < 100; i++) cms.add(i, 1);
        for (int i = 0; i < 100; i++) {
            TEST_ASSERT_GE(runner, cms.estimate(i), 1);
        }
    }
    runner.end_test();

    runner.begin_test("CountMinSketch frequency accuracy");
    {
        std::mt19937 rng(42);
        CountMinSketch cms(1000, 10, rng);
        // Insert item 42 one hundred times
        for (int i = 0; i < 100; i++) cms.add(42);
        TEST_ASSERT_GE(runner, cms.estimate(42), 100);
        // Insert noise items
        for (int i = 0; i < 1000; i++) cms.add(i, 1);
        // Estimate for 42 should still be >= 100
        TEST_ASSERT_GE(runner, cms.estimate(42), 100);
        // But might be overestimated due to collisions
        TEST_ASSERT_LE(runner, cms.estimate(42), 100 + 1000 + 10);
    }
    runner.end_test();

    runner.begin_test("HyperLogLog estimates cardinality");
    {
        for (int b = 4; b <= 14; b++) {
            HyperLogLog hll(b);
            int n = 1000;
            for (int i = 0; i < n; i++) hll.insert(i);
            double est = hll.estimate();
            // Within 10% for this size
            TEST_ASSERT_TRUE(runner, est > n * 0.5);
            TEST_ASSERT_TRUE(runner, est < n * 2.0);
        }
    }
    runner.end_test();

    runner.begin_test("HyperLogLog increasing estimates");
    {
        HyperLogLog hll(12);
        double prev = 0;
        for (int n = 100; n <= 10000; n += 100) {
            for (int i = n - 100; i < n; i++) hll.insert(i);
            double est = hll.estimate();
            TEST_ASSERT_TRUE(runner, est >= prev - 10.0); // non-decreasing roughly
            prev = est;
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: LazySelect
// ============================================================
void test_stress_lazy_select() {
    ral_test::TestRunner runner("Stress: LazySelect");

    runner.begin_test("lazy_select finds correct kth element");
    {
        std::mt19937 rng(42);
        for (int n : {50, 100, 200, 500}) {
            std::vector<int> S(n);
            for (int i = 0; i < n; i++) S[i] = i + 1;
            std::shuffle(S.begin(), S.end(), rng);

            std::vector<int> sorted = S;
            std::sort(sorted.begin(), sorted.end());

            for (int k : {1, n / 4, n / 2, 3 * n / 4, n}) {
                std::vector<int> copy = S;
                auto result = lazy_select(copy, k, rng);
                TEST_ASSERT_EQ(runner, result.value, sorted[k - 1]);
            }
        }
    }
    runner.end_test();

    runner.begin_test("lazy_select on reversed input");
    {
        std::mt19937 rng(42);
        int n = 200;
        std::vector<int> S(n);
        for (int i = 0; i < n; i++) S[i] = n - i;
        auto result = lazy_select(S, n / 2, rng);
        TEST_ASSERT_EQ(runner, result.value, n / 2);
    }
    runner.end_test();

    runner.begin_test("deterministic_select matches std::nth_element");
    {
        std::mt19937 rng(42);
        for (int t = 0; t < 50; t++) {
            int n = (t % 20) + 5;
            std::vector<int> S(n);
            for (int i = 0; i < n; i++) S[i] = (t * 13 + i * 7) % (n * 10) + 1;
            int k = (t % n) + 1;
            std::vector<int> sorted = S;
            std::sort(sorted.begin(), sorted.end());
            std::vector<int> copy = S;
            int result = deterministic_select(copy, k);
            TEST_ASSERT_EQ(runner, result, sorted[k - 1]);
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

int main() {
    test_stress_treap();
    test_stress_skip_list();
    test_stress_hash_table();
    test_stress_sketching();
    test_stress_lazy_select();
    return ral_test::run_all_suites() ? 0 : 1;
}
