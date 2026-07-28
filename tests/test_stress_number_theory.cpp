#include "test_framework.h"
#include "ral.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <set>

using namespace ral;

// ============================================================
// Stress: mod_pow
// ============================================================
void test_stress_mod_pow() {
    ral_test::TestRunner runner("Stress: mod_pow");
    runner.begin_test("mod_pow identity and overflow safety");
    // a^0 mod m == 1
    TEST_ASSERT_EQ(runner, mod_pow(7, 0, 13), 1);
    TEST_ASSERT_EQ(runner, mod_pow(1, 1000000, 998244353), 1);
    // a^1 mod m == a mod m
    TEST_ASSERT_EQ(runner, mod_pow(5, 1, 7), 5 % 7);
    // (a*b) mod m consistency: a^(b+c) = a^b * a^c mod m
    for (int t = 0; t < 200; t++) {
        long long a = (t * 7 + 3) % 1000 + 1;
        long long b = (t * 13 + 1) % 500 + 1;
        long long c = (t * 17 + 5) % 500 + 1;
        long long m = 998244353;
        long long lhs = mod_pow(a, b + c, m);
        long long rhs = (mod_pow(a, b, m) * mod_pow(a, c, m)) % m;
        TEST_ASSERT_EQ(runner, lhs, rhs);
    }
    runner.end_test();

    runner.begin_test("mod_pow large exponents");
    // Fermat's little theorem: a^(p-1) == 1 mod p for prime p, gcd(a,p)=1
    std::vector<long long> primes = {7, 13, 97, 101, 997, 1009, 104729};
    for (long long p : primes) {
        for (long long a = 1; a < std::min(p, 20LL); a++) {
            TEST_ASSERT_EQ(runner, mod_pow(a, p - 1, p), 1LL);
        }
    }
    runner.end_test();

    runner.begin_test("mod_pow mod 1");
    TEST_ASSERT_EQ(runner, mod_pow(123456, 789, 1), 0);
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: gcd and extended_gcd
// ============================================================
void test_stress_gcd() {
    ral_test::TestRunner runner("Stress: gcd & extended_gcd");
    runner.begin_test("gcd basic properties");
    for (int a = 1; a <= 200; a++) {
        for (int b = 1; b <= 200; b++) {
            long long g = gcd(a, b);
            TEST_ASSERT_TRUE(runner, g > 0);
            TEST_ASSERT_TRUE(runner, a % g == 0);
            TEST_ASSERT_TRUE(runner, b % g == 0);
        }
    }
    runner.end_test();

    runner.begin_test("extended_gcd correctness");
    for (int t = 0; t < 500; t++) {
        long long a = (t * 73 + 11) % 10000 + 1;
        long long b = (t * 97 + 19) % 10000 + 1;
        auto [g, x, y] = extended_gcd(a, b);
        TEST_ASSERT_EQ(runner, a * x + b * y, g);
        TEST_ASSERT_EQ(runner, g, gcd(a, b));
    }
    runner.end_test();

    runner.begin_test("gcd of consecutive numbers is 1");
    for (int n = 2; n <= 300; n++) {
        TEST_ASSERT_EQ(runner, gcd(n, n + 1), 1LL);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: mod_inverse
// ============================================================
void test_stress_mod_inverse() {
    ral_test::TestRunner runner("Stress: mod_inverse");
    runner.begin_test("mod_inverse roundtrip");
    for (long long p : {7LL, 13LL, 97LL, 101LL, 997LL, 104729LL}) {
        for (long long a = 1; a < p; a++) {
            long long inv = mod_inverse(a, p);
            TEST_ASSERT_EQ(runner, (a * inv) % p, 1LL);
        }
    }
    runner.end_test();

    runner.begin_test("mod_inverse large primes");
    for (long long a : {2, 3, 5, 7, 11, 13, 100, 999}) {
        long long p = 1000000007LL;
        long long inv = mod_inverse(a, p);
        TEST_ASSERT_EQ(runner, (a * inv) % p, 1LL);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: euler_totient
// ============================================================
void test_stress_euler_totient() {
    ral_test::TestRunner runner("Stress: euler_totient");
    runner.begin_test("euler_totient known values");
    // phi(1)=1, phi(2)=1, phi(p)=p-1 for prime p
    TEST_ASSERT_EQ(runner, euler_totient(1), 1LL);
    TEST_ASSERT_EQ(runner, euler_totient(2), 1LL);
    TEST_ASSERT_EQ(runner, euler_totient(3), 2LL);
    TEST_ASSERT_EQ(runner, euler_totient(10), 4LL);
    TEST_ASSERT_EQ(runner, euler_totient(12), 4LL);
    TEST_ASSERT_EQ(runner, euler_totient(36), 12LL);
    runner.end_test();

    runner.begin_test("euler_totient for primes");
    std::vector<long long> primes = {2, 3, 5, 7, 11, 13, 97, 101, 997, 1009};
    for (long long p : primes) {
        TEST_ASSERT_EQ(runner, euler_totient(p), p - 1);
    }
    runner.end_test();

    runner.begin_test("euler_totient multiplicative property (coprime)");
    // phi(a*b) = phi(a)*phi(b) when gcd(a,b)=1
    for (int a = 2; a <= 50; a++) {
        for (int b = 2; b <= 50; b++) {
            if (gcd(a, b) == 1) {
                TEST_ASSERT_EQ(runner, euler_totient(a * b), euler_totient(a) * euler_totient(b));
            }
        }
    }
    runner.end_test();

    runner.begin_test("euler_totient phi(n) <= n-1");
    for (int n = 1; n <= 500; n++) {
        TEST_ASSERT_LE(runner, euler_totient(n), n);
        if (n > 1) TEST_ASSERT_LE(runner, euler_totient(n), n - 1);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Chinese Remainder Theorem
// ============================================================
void test_stress_crt() {
    ral_test::TestRunner runner("Stress: CRT");
    runner.begin_test("CRT small systems");
    {
        std::vector<Congruence> system = {{2, 3}, {3, 5}, {2, 7}};
        long long x = chinese_remainder_theorem(system);
        TEST_ASSERT_EQ(runner, x % 3, 2LL);
        TEST_ASSERT_EQ(runner, x % 5, 3LL);
        TEST_ASSERT_EQ(runner, x % 7, 2LL);
    }
    runner.end_test();

    runner.begin_test("CRT random systems");
    std::vector<long long> small_primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    for (int t = 0; t < 200; t++) {
        int k = (t % 6) + 2;
        std::vector<Congruence> system;
        for (int i = 0; i < k; i++) {
            long long p = small_primes[i];
            long long a = (t * (i + 1) * 7 + 3) % p;
            system.push_back({a, p});
        }
        long long x = chinese_remainder_theorem(system);
        for (int i = 0; i < k; i++) {
            TEST_ASSERT_EQ(runner, ((x % system[i].n) + system[i].n) % system[i].n, system[i].a);
        }
    }
    runner.end_test();

    runner.begin_test("CRT single congruence");
    {
        std::vector<Congruence> system = {{5, 7}};
        long long x = chinese_remainder_theorem(system);
        TEST_ASSERT_EQ(runner, x % 7, 5LL);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: primality tests (trial, Fermat, Miller-Rabin, Solovay-Strassen)
// ============================================================
void test_stress_primality() {
    ral_test::TestRunner runner("Stress: Primality Tests");
    runner.begin_test("trial vs probabilistic on primes");
    std::vector<long long> known_primes = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,
        97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,
        997,1009,1013,1019,1021,1031,1033,1039,1049,1051,1061,1063,1069,1087,1091,1093};
    for (long long p : known_primes) {
        TEST_ASSERT_TRUE(runner, is_prime_trial(p));
        TEST_ASSERT_TRUE(runner, fermat_primality_test(p, 20));
        TEST_ASSERT_TRUE(runner, miller_rabin(p, 20));
        TEST_ASSERT_TRUE(runner, solovay_strassen(p, 20));
    }
    runner.end_test();

    runner.begin_test("composites detected correctly");
    std::vector<long long> composites = {4,6,8,9,10,12,14,15,16,18,20,21,22,24,25,26,27,28,
        100,101*103,997*1009};
    for (long long c : composites) {
        TEST_ASSERT_TRUE(runner, !is_prime_trial(c));
        TEST_ASSERT_TRUE(runner, !miller_rabin(c, 20));
    }
    runner.end_test();

    runner.begin_test("Carmichael number 561");
    TEST_ASSERT_TRUE(runner, !is_prime_trial(561));
    TEST_ASSERT_TRUE(runner, !miller_rabin(561, 20));
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: quadratic residue, Legendre, Jacobi
// ============================================================
void test_stress_symbol() {
    ral_test::TestRunner runner("Stress: Jacobi/Legendre/QR");
    runner.begin_test("Legendre symbol properties");
    for (long long p : {3LL, 5LL, 7LL, 11LL, 13LL, 17LL, 19LL, 23LL, 97LL}) {
        int qr_count = 0;
        for (long long a = 1; a < p; a++) {
            int ls = legendre_symbol(a, p);
            TEST_ASSERT_TRUE(runner, ls == 1 || ls == -1);
            if (ls == 1) qr_count++;
        }
        // Exactly (p-1)/2 quadratic residues
        TEST_ASSERT_EQ(runner, qr_count, (p - 1) / 2);
    }
    runner.end_test();

    runner.begin_test("Jacobi symbol for prime equals Legendre");
    for (long long p : {3LL, 7LL, 11LL, 13LL, 97LL}) {
        for (long long a = 1; a < p; a++) {
            TEST_ASSERT_EQ(runner, jacobi_symbol(a, p), legendre_symbol(a, p));
        }
    }
    runner.end_test();

    runner.begin_test("Jacobi symbol multiplicative in first argument");
    for (long long n : {15LL, 21LL, 33LL, 35LL, 91LL}) {
        for (long long a = 1; a < n; a += 2) {
            for (long long b = 1; b < n; b += 2) {
                int js = jacobi_symbol((a * b) % n, n);
                int js_a = jacobi_symbol(a, n);
                int js_b = jacobi_symbol(b, n);
                TEST_ASSERT_EQ(runner, js, js_a * js_b);
            }
        }
    }
    runner.end_test();

    runner.begin_test("is_quadratic_residue matches Legendre");
    for (long long p : {7LL, 13LL, 17LL, 97LL}) {
        for (long long a = 0; a < p; a++) {
            bool qr = is_quadratic_residue(a, p);
            if (a == 0) {
                TEST_ASSERT_TRUE(runner, qr);
            } else {
                TEST_ASSERT_TRUE(runner, qr == (legendre_symbol(a, p) == 1));
            }
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: RSA
// ============================================================
void test_stress_rsa() {
    ral_test::TestRunner runner("Stress: RSA");
    runner.begin_test("RSA encrypt/decrypt roundtrip");
    struct TestCase { long long p, q, e; };
    std::vector<TestCase> cases = {
        {61, 53, 17}, {11, 13, 7}, {17, 19, 5}, {101, 103, 7}, {97, 89, 5}
    };
    for (auto [p, q, e] : cases) {
        auto [pub_e, priv_d, n] = generate_rsa_keypair(p, q, e);
        for (long long msg : {0LL, 1LL, 42LL, 100LL, n - 1, n / 2}) {
            long long cipher = rsa_encrypt(msg, pub_e, n);
            long long plain = rsa_decrypt(cipher, priv_d, n);
            TEST_ASSERT_EQ(runner, plain, msg);
        }
    }
    runner.end_test();

    runner.begin_test("RSA key generation validity");
    for (auto [p, q, e] : cases) {
        auto [pub_e, priv_d, n] = generate_rsa_keypair(p, q, e);
        TEST_ASSERT_EQ(runner, n, p * q);
        TEST_ASSERT_EQ(runner, (pub_e * priv_d) % ((p - 1) * (q - 1)), 1LL);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: polynomial identity testing
// ============================================================
void test_stress_polynomial() {
    ral_test::TestRunner runner("Stress: Polynomial");
    runner.begin_test("Polynomial evaluate consistency");
    long long p = 97;
    for (int t = 0; t < 300; t++) {
        Polynomial f({(long long)(t * 3 + 1) % p, (long long)(t * 7 + 2) % p, (long long)(t * 11 + 3) % p});
        Polynomial g({(long long)(t * 5 + 4) % p, (long long)(t * 13 + 6) % p});
        Polynomial prod = f.multiply(g, p);
        long long x = t % p;
        long long fval = f.evaluate(x, p);
        long long gval = g.evaluate(x, p);
        long long prodval = prod.evaluate(x, p);
        TEST_ASSERT_EQ(runner, (fval * gval) % p, prodval);
    }
    runner.end_test();

    runner.begin_test("polynomial_identity_test same polynomials");
    for (int t = 0; t < 100; t++) {
        Polynomial a({(long long)(t * 3 + 1) % 97, (long long)(t * 7 + 2) % 97});
        TEST_ASSERT_TRUE(runner, polynomial_identity_test(a, a, 97, 30));
    }
    runner.end_test();

    runner.begin_test("polynomial_identity_test different polynomials");
    for (int t = 0; t < 100; t++) {
        Polynomial a({1, 2, 3});
        Polynomial b({3, 2, 1});
        TEST_ASSERT_TRUE(runner, !polynomial_identity_test(a, b, 97, 30));
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: polynomial_chapter8
// ============================================================
void test_stress_poly_ch8() {
    ral_test::TestRunner runner("Stress: Polynomial Ch8");
    std::mt19937 rng(42);

    runner.begin_test("poly_eval correctness");
    long long mod = 1000000007LL;
    for (int t = 0; t < 300; t++) {
        std::vector<int> coeffs = {(t * 3 + 1) % 1000, (t * 7 + 2) % 1000, (t * 11 + 3) % 1000};
        long long x = (t * 5 + 4) % 100;
        long long val = poly_eval(coeffs, x, mod);
        // Verify by direct computation
        long long expected = 0;
        long long xpow = 1;
        for (int c : coeffs) {
            expected = (expected + (long long)c % mod * xpow % mod) % mod;
            xpow = xpow * x % mod;
        }
        TEST_ASSERT_EQ(runner, val, expected);
    }
    runner.end_test();

    runner.begin_test("polynomial_identity_test ch8");
    std::vector<int> p1 = {1, 2, 1};
    std::vector<int> q1 = {1, 2, 1};
    TEST_ASSERT_TRUE(runner, polynomial_identity_test(p1, q1, 10000, 20, rng));

    std::vector<int> p2 = {1, 2, 1};
    std::vector<int> q2 = {1, 3, 1};
    TEST_ASSERT_TRUE(runner, !polynomial_identity_test(p2, q2, 10000, 20, rng));
    runner.end_test();

    runner.begin_test("poly_degree");
    TEST_ASSERT_EQ(runner, poly_degree({3, 0, 2, 1}), 3);
    TEST_ASSERT_EQ(runner, poly_degree({1, 2, 1}), 2);
    TEST_ASSERT_EQ(runner, poly_degree({5}), 0);
    TEST_ASSERT_EQ(runner, poly_degree({0, 0, 0}), -1);
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: randomized linear algebra
// ============================================================
void test_stress_randla() {
    ral_test::TestRunner runner("Stress: RandomizedLinearAlgebra");

    runner.begin_test("transpose roundtrip");
    using Matrix = std::vector<std::vector<double>>;
    for (int t = 0; t < 50; t++) {
        int m = (t % 5) + 1, n = (t % 4) + 1;
        Matrix A(m, std::vector<double>(n));
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                A[i][j] = t * 100 + i * 10 + j;
        Matrix AT = RandomizedLA::transpose(A);
        Matrix ATT = RandomizedLA::transpose(AT);
        TEST_ASSERT_EQ(runner, (int)ATT.size(), m);
        TEST_ASSERT_EQ(runner, (int)ATT[0].size(), n);
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                TEST_ASSERT_NEAR(runner, ATT[i][j], A[i][j], 1e-9);
    }
    runner.end_test();

    runner.begin_test("multiply identity");
    for (int n = 1; n <= 8; n++) {
        Matrix I(n, std::vector<double>(n, 0.0));
        for (int i = 0; i < n; i++) I[i][i] = 1.0;
        Matrix A(n, std::vector<double>(n));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                A[i][j] = (i * 7 + j * 3 + 1) % 20;
        Matrix AI = RandomizedLA::multiply(A, I);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                TEST_ASSERT_NEAR(runner, AI[i][j], A[i][j], 1e-9);
    }
    runner.end_test();

    runner.begin_test("orthonormalize columns are orthonormal");
    for (int t = 0; t < 30; t++) {
        int m = (t % 5) + 3, r = (t % 3) + 1;
        if (r > m) continue;
        Matrix Y(m, std::vector<double>(r));
        std::mt19937 rng(t);
        std::normal_distribution<double> norm(0.0, 1.0);
        for (int i = 0; i < m; i++)
            for (int j = 0; j < r; j++)
                Y[i][j] = norm(rng);
        Matrix Q = RandomizedLA::orthonormalize(Y);
        // Check orthonormality: Q^T Q ~ I
        for (int i = 0; i < r; i++) {
            for (int j = 0; j < r; j++) {
                double dot = 0;
                for (int k = 0; k < m; k++) dot += Q[k][i] * Q[k][j];
                double expected = (i == j) ? 1.0 : 0.0;
                TEST_ASSERT_NEAR(runner, dot, expected, 1e-6);
            }
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: martingales
// ============================================================
void test_stress_martingales() {
    ral_test::TestRunner runner("Stress: Martingales");
    runner.begin_test("Azuma-Hoeffding bound is valid probability");
    for (int n : {10, 50, 100, 500}) {
        for (double c : {0.1, 0.5, 1.0, 2.0}) {
            for (double t = 0.1; t <= 5.0; t += 0.5) {
                double bound = azuma_hoeffding_bound(n, c, t);
                TEST_ASSERT_TRUE(runner, bound >= 0.0);
                TEST_ASSERT_TRUE(runner, bound <= 2.0);
            }
        }
    }
    runner.end_test();

    runner.begin_test("Azuma-Hoeffding decreases with t");
    for (int n = 100; n <= 500; n += 100) {
        double prev = 3.0;
        for (double t = 0.5; t <= 10.0; t += 0.5) {
            double bound = azuma_hoeffding_bound(n, 1.0, t);
            TEST_ASSERT_TRUE(runner, bound <= prev + 1e-12);
            prev = bound;
        }
    }
    runner.end_test();

    runner.begin_test("Non-uniform differences bound");
    for (int n = 10; n <= 100; n += 10) {
        std::vector<double> c(n);
        for (int i = 0; i < n; i++) c[i] = 1.0 / (i + 1);
        double bound = azuma_hoeffding_bound(c, 3.0);
        TEST_ASSERT_TRUE(runner, bound >= 0.0);
        TEST_ASSERT_TRUE(runner, bound <= 2.0);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: probabilistic recurrence / randomized find
// ============================================================
void test_stress_find() {
    ral_test::TestRunner runner("Stress: RandomizedFind");
    runner.begin_test("randomized_find correctness");
    for (int t = 0; t < 200; t++) {
        int n = (t % 20) + 5;
        std::vector<int> arr(n);
        for (int i = 0; i < n; i++) arr[i] = (t * 13 + i * 7) % (n * 10) + 1;
        int k = (t % n) + 1;
        std::vector<int> sorted = arr;
        std::sort(sorted.begin(), sorted.end());
        int result = randomized_find(arr, k);
        TEST_ASSERT_EQ(runner, result, sorted[k - 1]);
    }
    runner.end_test();

    runner.begin_test("randomized_find on sorted input");
    for (int n = 5; n <= 100; n += 5) {
        std::vector<int> arr(n);
        std::iota(arr.begin(), arr.end(), 1);
        for (int k = 1; k <= n; k++) {
            std::vector<int> copy = arr;
            int result = randomized_find(copy, k);
            TEST_ASSERT_EQ(runner, result, k);
        }
    }
    runner.end_test();

    runner.begin_test("randomized_find on identical elements");
    std::vector<int> identical(50, 7);
    TEST_ASSERT_EQ(runner, randomized_find(identical, 1), 7);
    TEST_ASSERT_EQ(runner, randomized_find(identical, 25), 7);
    TEST_ASSERT_EQ(runner, randomized_find(identical, 50), 7);
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: linear programming
// ============================================================
void test_stress_lp() {
    ral_test::TestRunner runner("Stress: LinearProgramming");
    runner.begin_test("LP: minimize x+y >= 0");
    for (int t = 0; t < 100; t++) {
        std::vector<std::vector<double>> A = {
            {-1.0, 0.0}, {0.0, -1.0}, {1.0, 1.0}
        };
        std::vector<double> b = {0.0, 0.0, 10.0 + t};
        std::vector<double> c = {1.0, 1.0};
        auto res = solve_lp(A, b, c, 2);
        if (res.feasible) {
            TEST_ASSERT_TRUE(runner, res.x[0] >= -0.01);
            TEST_ASSERT_TRUE(runner, res.x[1] >= -0.01);
            TEST_ASSERT_TRUE(runner, res.x[0] + res.x[1] <= 10.0 + t + 0.01);
        }
    }
    runner.end_test();

    runner.begin_test("LP: maximize x subject to x <= 5");
    std::vector<std::vector<double>> A = {{1.0}, {-1.0}};
    std::vector<double> b = {5.0, 0.0};
    std::vector<double> c = {-1.0}; // minimize -x = maximize x
    auto res = solve_lp(A, b, c, 1);
    if (res.feasible) {
        TEST_ASSERT_NEAR(runner, res.x[0], 5.0, 0.1);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

int main() {
    test_stress_mod_pow();
    test_stress_gcd();
    test_stress_mod_inverse();
    test_stress_euler_totient();
    test_stress_crt();
    test_stress_primality();
    test_stress_symbol();
    test_stress_rsa();
    test_stress_polynomial();
    test_stress_poly_ch8();
    test_stress_randla();
    test_stress_martingales();
    test_stress_find();
    test_stress_lp();
    return ral_test::run_all_suites() ? 0 : 1;
}
