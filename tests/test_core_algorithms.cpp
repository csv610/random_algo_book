#include "test_framework.h"
#include "ral.h"

#include <random>
#include <vector>
#include <algorithm>

// ---- Number Theory ----

void test_mod_pow() {
    ral_test::TestRunner runner("mod_pow");
    runner.begin_test("mod_pow 2^10 mod 1000 = 24");
    TEST_ASSERT_EQ(runner, ral::mod_pow(2, 10, 1000), 24);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_mod_pow2() {
    ral_test::TestRunner runner("mod_pow 3^13 mod 11");
    runner.begin_test("3^13 mod 11 = 5");
    TEST_ASSERT_EQ(runner, ral::mod_pow(3, 13, 11), 5);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_mod_pow3() {
    ral_test::TestRunner runner("mod_pow 0^0 mod 1");
    runner.begin_test("0^0 mod 1 = 0");
    TEST_ASSERT_EQ(runner, ral::mod_pow(0, 0, 1), 0);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_gcd() {
    ral_test::TestRunner runner("gcd");
    runner.begin_test("gcd(12,8)=4");
    TEST_ASSERT_EQ(runner, ral::gcd(12, 8), 4);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_gcd2() {
    ral_test::TestRunner runner("gcd coprime");
    runner.begin_test("gcd(17,13)=1");
    TEST_ASSERT_EQ(runner, ral::gcd(17, 13), 1);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_gcd3() {
    ral_test::TestRunner runner("gcd with zero");
    runner.begin_test("gcd(0,5)=5");
    TEST_ASSERT_EQ(runner, ral::gcd(0, 5), 5);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_extended_gcd() {
    ral_test::TestRunner runner("extended_gcd");
    runner.begin_test("extended_gcd(12,8)");
    {
        auto [g, x, y] = ral::extended_gcd(12, 8);
        TEST_ASSERT_EQ(runner, g, 4);
        TEST_ASSERT_EQ(runner, 12 * x + 8 * y, g);
    }
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_extended_gcd2() {
    ral_test::TestRunner runner("extended_gcd(35,15)");
    runner.begin_test("ax + by = gcd");
    {
        auto [g, x, y] = ral::extended_gcd(35, 15);
        TEST_ASSERT_EQ(runner, g, 5);
        TEST_ASSERT_EQ(runner, 35 * x + 15 * y, g);
    }
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_extended_gcd3() {
    ral_test::TestRunner runner("extended_gcd(7,3)");
    runner.begin_test("ax + by = gcd");
    {
        auto [g, x, y] = ral::extended_gcd(7, 3);
        TEST_ASSERT_EQ(runner, g, 1);
        TEST_ASSERT_EQ(runner, 7 * x + 3 * y, g);
    }
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_mod_inverse() {
    ral_test::TestRunner runner("mod_inverse");
    runner.begin_test("inv(3,7)=5");
    TEST_ASSERT_EQ(runner, ral::mod_inverse(3, 7), 5);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_mod_inverse2() {
    ral_test::TestRunner runner("mod_inverse(10,17)");
    runner.begin_test("inv(10,17)=12");
    TEST_ASSERT_EQ(runner, ral::mod_inverse(10, 17), 12);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_is_prime() {
    ral_test::TestRunner runner("is_prime_trial");
    runner.begin_test("primes");
    std::vector<long long> primes = {2, 3, 5, 7, 11, 13, 97};
    for (long long p : primes) {
        TEST_ASSERT_TRUE_MSG(runner, ral::is_prime_trial(p),
                             std::to_string(p) + " should be prime");
    }
    runner.end_test();

    runner.begin_test("composites");
    std::vector<long long> composites = {1, 4, 6, 8, 9, 15, 100};
    for (long long c : composites) {
        TEST_ASSERT_TRUE_MSG(runner, !ral::is_prime_trial(c),
                             std::to_string(c) + " should not be prime");
    }
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_euler_totient() {
    ral_test::TestRunner runner("euler_totient");
    runner.begin_test("totient values");
    TEST_ASSERT_EQ(runner, ral::euler_totient(1), 1);
    TEST_ASSERT_EQ(runner, ral::euler_totient(2), 1);
    TEST_ASSERT_EQ(runner, ral::euler_totient(6), 2);
    TEST_ASSERT_EQ(runner, ral::euler_totient(10), 4);
    TEST_ASSERT_EQ(runner, ral::euler_totient(12), 4);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ---- Convex Hull ----

void test_cross_product() {
    ral_test::TestRunner runner("cross_product");
    runner.begin_test("collinear");
    TEST_ASSERT_NEAR(runner, ral::cross_product({0,0}, {1,0}, {2,0}), 0.0, 1e-9);
    runner.end_test();

    runner.begin_test("counter-clockwise");
    TEST_ASSERT_GT(runner, ral::cross_product({0,0}, {1,0}, {0,1}), 0.0);
    runner.end_test();

    runner.begin_test("clockwise");
    TEST_ASSERT_LT(runner, ral::cross_product({0,0}, {0,1}, {1,0}), 0.0);
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_dist2() {
    ral_test::TestRunner runner("dist2");
    runner.begin_test("distance (0,0) to (3,4) = 25");
    TEST_ASSERT_NEAR(runner, ral::dist2({0,0}, {3,4}), 25.0, 1e-9);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_convex_hull_square() {
    ral_test::TestRunner runner("convex_hull square");
    runner.begin_test("unit square returns 4 hull points");
    std::vector<ral::Point2D> pts = {{0,0}, {1,0}, {1,1}, {0,1}};
    auto hull = ral::convex_hull(pts);
    TEST_ASSERT_EQ(runner, (int)hull.size(), 4);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_convex_hull_triangle() {
    ral_test::TestRunner runner("convex_hull triangle");
    runner.begin_test("triangle returns 3 hull points");
    std::vector<ral::Point2D> pts = {{0,0}, {1,0}, {0,1}};
    auto hull = ral::convex_hull(pts);
    TEST_ASSERT_EQ(runner, (int)hull.size(), 3);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_gift_wrap() {
    ral_test::TestRunner runner("gift_wrap_hull");
    runner.begin_test("square");
    {
        std::vector<ral::Point2D> pts = {{0,0}, {1,0}, {1,1}, {0,1}};
        auto hull = ral::gift_wrap_hull(pts);
        TEST_ASSERT_EQ(runner, (int)hull.size(), 4);
    }
    runner.end_test();

    runner.begin_test("triangle");
    {
        std::vector<ral::Point2D> pts = {{0,0}, {1,0}, {0,1}};
        auto hull = ral::gift_wrap_hull(pts);
        TEST_ASSERT_EQ(runner, (int)hull.size(), 3);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ---- Chernoff Bounds ----

void test_chernoff_upper_positive() {
    ral_test::TestRunner runner("chernoff_upper positive");
    runner.begin_test("chernoff_upper(mu=10, delta=1) > 0");
    double val = ral::chernoff_upper(10.0, 1.0);
    TEST_ASSERT_GT(runner, val, 0.0);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_chernoff_monotone() {
    ral_test::TestRunner runner("chernoff monotone");
    runner.begin_test("monotone decreasing in delta");
    double v1 = ral::chernoff_upper(10.0, 0.5);
    double v2 = ral::chernoff_upper(10.0, 2.0);
    TEST_ASSERT_GT(runner, v1, v2);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_chernoff_simplified_looser() {
    ral_test::TestRunner runner("chernoff simplified");
    runner.begin_test("simplified >= upper");
    double upper = ral::chernoff_upper(10.0, 0.5);
    double simplified = ral::chernoff_simplified(10.0, 0.5);
    TEST_ASSERT_GE(runner, simplified, upper);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ---- Treap ----

void test_treap_insert_search() {
    ral_test::TestRunner runner("treap insert/search");
    runner.begin_test("insert and search");
    std::mt19937 rng(42);
    ral::Treap treap(rng);
    for (int i = 1; i <= 20; i++) {
        treap.insert(i);
    }
    for (int i = 1; i <= 20; i++) {
        TEST_ASSERT_TRUE_MSG(runner, treap.search(i),
                             "search(" + std::to_string(i) + ") should find");
    }
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_treap_remove() {
    ral_test::TestRunner runner("treap remove");
    runner.begin_test("remove then search fails");
    std::mt19937 rng(42);
    ral::Treap treap(rng);
    for (int i = 1; i <= 10; i++) {
        treap.insert(i);
    }
    treap.remove(5);
    TEST_ASSERT_TRUE_MSG(runner, !treap.search(5), "5 should be gone");
    TEST_ASSERT_TRUE_MSG(runner, treap.search(4), "4 should still be there");
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_treap_inorder() {
    ral_test::TestRunner runner("treap inorder");
    runner.begin_test("inorder is sorted");
    std::mt19937 rng(42);
    ral::Treap treap(rng);
    for (int i : {5, 3, 8, 1, 4, 7, 9, 2, 6}) {
        treap.insert(i);
    }
    auto sorted = treap.inorder();
    TEST_ASSERT_TRUE(runner, std::is_sorted(sorted.begin(), sorted.end()));
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_treap_verify() {
    ral_test::TestRunner runner("treap verify");
    runner.begin_test("BST and heap properties");
    std::mt19937 rng(42);
    ral::Treap treap(rng);
    for (int i = 1; i <= 15; i++) {
        treap.insert(i);
    }
    TEST_ASSERT_TRUE(runner, treap.verify_bst());
    TEST_ASSERT_TRUE(runner, treap.verify_heap());
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_treap_size() {
    ral_test::TestRunner runner("treap size");
    runner.begin_test("size tracks correctly");
    std::mt19937 rng(42);
    ral::Treap treap(rng);
    TEST_ASSERT_EQ(runner, treap.size(), 0);
    treap.insert(1);
    TEST_ASSERT_EQ(runner, treap.size(), 1);
    treap.insert(2);
    treap.insert(3);
    TEST_ASSERT_EQ(runner, treap.size(), 3);
    treap.remove(2);
    TEST_ASSERT_EQ(runner, treap.size(), 2);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ---- Skip List ----

void test_skiplist_insert_search() {
    ral_test::TestRunner runner("skiplist insert/search");
    runner.begin_test("insert and search");
    std::mt19937 rng(42);
    ral::SkipList sl(rng);
    for (int i = 1; i <= 20; i++) {
        sl.insert(i);
    }
    for (int i = 1; i <= 20; i++) {
        TEST_ASSERT_TRUE_MSG(runner, sl.search(i),
                             "search(" + std::to_string(i) + ") should find");
    }
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_skiplist_size() {
    ral_test::TestRunner runner("skiplist size");
    runner.begin_test("size correct after inserts");
    std::mt19937 rng(42);
    ral::SkipList sl(rng);
    for (int i = 1; i <= 15; i++) {
        sl.insert(i);
    }
    TEST_ASSERT_EQ(runner, sl.size(), 15);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_skiplist_not_found() {
    ral_test::TestRunner runner("skiplist not found");
    runner.begin_test("search for non-existent returns false");
    std::mt19937 rng(42);
    ral::SkipList sl(rng);
    for (int i = 1; i <= 10; i++) {
        sl.insert(i);
    }
    TEST_ASSERT_TRUE(runner, !sl.search(99));
    TEST_ASSERT_TRUE(runner, !sl.search(0));
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ---- Rabin-Karp ----

void test_rabin_karp_aba() {
    ral_test::TestRunner runner("rabin_karp ABA");
    runner.begin_test("search ABA in ABABABA");
    auto result = ral::rabin_karp_search("ABABABA", "ABA");
    std::vector<int> expected = {0, 2, 4};
    TEST_ASSERT_EQ(runner, (int)result.size(), (int)expected.size());
    TEST_ASSERT_TRUE(runner, result == expected);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_rabin_karp_aaaa() {
    ral_test::TestRunner runner("rabin_karp AAAA");
    runner.begin_test("search AA in AAAA");
    auto result = ral::rabin_karp_search("AAAA", "AA");
    std::vector<int> expected = {0, 1, 2};
    TEST_ASSERT_EQ(runner, (int)result.size(), (int)expected.size());
    TEST_ASSERT_TRUE(runner, result == expected);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_rabin_karp_no_match() {
    ral_test::TestRunner runner("rabin_karp no match");
    runner.begin_test("search WORLD in HELLO");
    auto result = ral::rabin_karp_search("HELLO", "WORLD");
    TEST_ASSERT_TRUE(runner, result.empty());
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_rabin_karp_single() {
    ral_test::TestRunner runner("rabin_karp single char");
    runner.begin_test("search A in A");
    auto result = ral::rabin_karp_search("A", "A");
    std::vector<int> expected = {0};
    TEST_ASSERT_TRUE(runner, result == expected);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ---- Freivalds ----

void test_freivalds_correct() {
    ral_test::TestRunner runner("freivalds correct");
    runner.begin_test("A*B=C correct product");
    std::mt19937 rng(42);
    std::vector<std::vector<long long>> A = {{1, 2}, {3, 4}};
    std::vector<std::vector<long long>> B = {{5, 6}, {7, 8}};
    std::vector<std::vector<long long>> C = {{19, 22}, {43, 50}};
    TEST_ASSERT_TRUE(runner, ral::freivalds_verify(A, B, C, 20, rng));
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_freivalds_wrong() {
    ral_test::TestRunner runner("freivalds wrong");
    runner.begin_test("A*B=C wrong product");
    std::mt19937 rng(42);
    std::vector<std::vector<long long>> A = {{1, 2}, {3, 4}};
    std::vector<std::vector<long long>> B = {{5, 6}, {7, 8}};
    std::vector<std::vector<long long>> C = {{20, 22}, {43, 50}};
    TEST_ASSERT_TRUE(runner, !ral::freivalds_verify(A, B, C, 20, rng));
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ---- Random Projection ----

void test_rp_distance_identical() {
    ral_test::TestRunner runner("random_projection identical");
    runner.begin_test("squared_distance identical vectors = 0");
    std::vector<double> v = {1.0, 2.0, 3.0};
    TEST_ASSERT_NEAR(runner, ral::RandomProjection::squared_distance(v, v), 0.0, 1e-12);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_rp_distance_3_4_0() {
    ral_test::TestRunner runner("random_projection distance");
    runner.begin_test("squared_distance (0,0,0) to (3,4,0) = 25");
    std::vector<double> v1 = {0.0, 0.0, 0.0};
    std::vector<double> v2 = {3.0, 4.0, 0.0};
    TEST_ASSERT_NEAR(runner, ral::RandomProjection::squared_distance(v1, v2), 25.0, 1e-9);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_rp_project_dimension() {
    ral_test::TestRunner runner("random_projection dimension");
    runner.begin_test("project returns correct dimension");
    std::mt19937 rng(42);
    ral::RandomProjection rp(10, 3, rng);
    std::vector<double> point(10, 1.0);
    auto projected = rp.project(point);
    TEST_ASSERT_EQ(runner, (int)projected.size(), 3);
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ---- Two Point ----

void test_two_point_rp() {
    ral_test::TestRunner runner("two_point RP");
    runner.begin_test("RP_algorithm x_in_L=false always returns false");
    for (int r = 0; r < 100; r++) {
        TEST_ASSERT_TRUE(runner, !ral::RP_algorithm(0, r, false, 500));
    }
    runner.end_test();
    runner.begin_test("RP_algorithm x_in_L=true returns r < n_half");
    for (int r = 0; r < 100; r++) {
        TEST_ASSERT_EQ(runner, ral::RP_algorithm(0, r, true, 500), r < 500);
    }
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ---- Coupon Collector ----

void test_coupon_n1() {
    ral_test::TestRunner runner("coupon_collector n=1");
    runner.begin_test("n=1 always returns exactly 1");
    std::mt19937 rng(42);
    for (int t = 0; t < 1000; t++) {
        TEST_ASSERT_EQ(runner, ral::simulate_coupon_collector(1, rng), 1);
    }
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

void test_coupon_n2() {
    ral_test::TestRunner runner("coupon_collector n=2");
    runner.begin_test("n=2 always returns >= 2");
    std::mt19937 rng(42);
    for (int t = 0; t < 1000; t++) {
        TEST_ASSERT_GE(runner, ral::simulate_coupon_collector(2, rng), 2);
    }
    runner.end_test();
    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ---- main ----

int main() {
    // Number theory
    test_mod_pow();
    test_mod_pow2();
    test_mod_pow3();
    test_gcd();
    test_gcd2();
    test_gcd3();
    test_extended_gcd();
    test_extended_gcd2();
    test_extended_gcd3();
    test_mod_inverse();
    test_mod_inverse2();
    test_is_prime();
    test_euler_totient();

    // Convex hull
    test_cross_product();
    test_dist2();
    test_convex_hull_square();
    test_convex_hull_triangle();
    test_gift_wrap();

    // Chernoff
    test_chernoff_upper_positive();
    test_chernoff_monotone();
    test_chernoff_simplified_looser();

    // Treap
    test_treap_insert_search();
    test_treap_remove();
    test_treap_inorder();
    test_treap_verify();
    test_treap_size();

    // Skip list
    test_skiplist_insert_search();
    test_skiplist_size();
    test_skiplist_not_found();

    // Rabin-Karp
    test_rabin_karp_aba();
    test_rabin_karp_aaaa();
    test_rabin_karp_no_match();
    test_rabin_karp_single();

    // Freivalds
    test_freivalds_correct();
    test_freivalds_wrong();

    // Random projection
    test_rp_distance_identical();
    test_rp_distance_3_4_0();
    test_rp_project_dimension();

    // Two point
    test_two_point_rp();

    // Coupon collector
    test_coupon_n1();
    test_coupon_n2();

    bool all_pass = ral_test::run_all_suites();
    return all_pass ? 0 : 1;
}
