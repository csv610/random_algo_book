#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <functional>
#include <sstream>

namespace ral_test {

struct TestResult {
    std::string name;
    int assertions;
    int passed;
    int failed;
    std::vector<std::string> failures;
};

struct SuiteResult {
    std::string suite_name;
    std::vector<TestResult> tests;
    int total_assertions() const { int s=0; for(auto& t:tests) s+=t.assertions; return s; }
    int total_passed() const { int s=0; for(auto& t:tests) s+=t.passed; return s; }
    int total_failed() const { int s=0; for(auto& t:tests) s+=t.failed; return s; }
};

class TestRunner {
    SuiteResult suite_;
    TestResult current_;
    bool in_test_ = false;

public:
    explicit TestRunner(const std::string& name) : suite_{name} {}

    void begin_test(const std::string& name) {
        current_ = {name, 0, 0, 0, {}};
        in_test_ = true;
    }

    void end_test() {
        if (in_test_) {
            suite_.tests.push_back(current_);
            in_test_ = false;
            if (current_.failed > 0) {
                std::cout << "  FAIL: " << current_.name
                          << " (" << current_.failed << "/" << current_.assertions << " failed)\n";
                for (auto& f : current_.failures) std::cout << "    " << f << "\n";
            } else {
                std::cout << "  OK: " << current_.name
                          << " (" << current_.assertions << " assertions)\n";
            }
        }
    }

    void record_pass() { current_.assertions++; current_.passed++; }
    void record_fail(const std::string& msg) {
        current_.assertions++; current_.failed++;
        current_.failures.push_back(msg);
    }

    void report() const {
        std::cout << "\n=== " << suite_.suite_name << " ===\n";
        std::cout << "  Tests: " << suite_.tests.size()
                  << "  Assertions: " << suite_.total_assertions()
                  << "  Passed: " << suite_.total_passed()
                  << "  Failed: " << suite_.total_failed() << "\n";
    }

    bool all_passed() const { return suite_.total_failed() == 0; }
    const SuiteResult& result() const { return suite_; }
};

// Global registry
inline std::vector<SuiteResult>& all_results() {
    static std::vector<SuiteResult> r;
    return r;
}

inline bool run_all_suites() {
    int total_suites = 0, total_failed = 0;
    for (auto& s : all_results()) {
        total_suites++;
        total_failed += s.total_failed();
        std::cout << "\n=== " << s.suite_name << " ===\n";
        std::cout << "  " << s.tests.size() << " tests, "
                  << s.total_assertions() << " assertions, "
                  << s.total_passed() << " passed, "
                  << s.total_failed() << " failed\n";
        for (auto& t : s.tests) {
            if (t.failed > 0) {
                std::cout << "  FAIL: " << t.name << "\n";
                for (auto& f : t.failures) std::cout << "    " << f << "\n";
            }
        }
    }
    std::cout << "\n========================================\n";
    std::cout << "TOTAL: " << total_suites << " suites, "
              << (total_failed == 0 ? "ALL PASSED" : std::to_string(total_failed) + " FAILURES") << "\n";
    std::cout << "========================================\n";
    return total_failed == 0;
}

// --- Assertion helpers ---

inline void assert_true_impl(TestRunner& runner, bool cond,
                              const std::string& expr, const char* file, int line) {
    if (cond) { runner.record_pass(); }
    else {
        std::ostringstream oss;
        oss << file << ":" << line << ": ASSERT_TRUE(" << expr << ") failed";
        runner.record_fail(oss.str());
    }
}

inline void assert_eq_impl(TestRunner& runner, long long a, long long b,
                            const std::string& expr_a, const std::string& expr_b,
                            const char* file, int line) {
    if (a == b) { runner.record_pass(); }
    else {
        std::ostringstream oss;
        oss << file << ":" << line << ": ASSERT_EQ(" << expr_a << ", " << expr_b
            << ") failed: " << a << " != " << b;
        runner.record_fail(oss.str());
    }
}

inline void assert_near_impl(TestRunner& runner, double a, double b, double tol,
                              const std::string& expr_a, const std::string& expr_b,
                              const char* file, int line) {
    if (std::abs(a - b) <= tol) { runner.record_pass(); }
    else {
        std::ostringstream oss;
        oss << file << ":" << line << ": ASSERT_NEAR(" << expr_a << ", " << expr_b
            << ", " << tol << ") failed: " << a << " != " << b
            << " (diff=" << std::abs(a - b) << ")";
        runner.record_fail(oss.str());
    }
}

} // namespace ral_test

// --- Macros ---

#define TEST_ASSERT_TRUE(runner, expr) \
    ral_test::assert_true_impl(runner, (expr), #expr, __FILE__, __LINE__)

#define TEST_ASSERT_EQ(runner, a, b) \
    ral_test::assert_eq_impl(runner, (long long)(a), (long long)(b), #a, #b, __FILE__, __LINE__)

#define TEST_ASSERT_NEAR(runner, a, b, tol) \
    ral_test::assert_near_impl(runner, (double)(a), (double)(b), (double)(tol), #a, #b, __FILE__, __LINE__)

#define TEST_ASSERT_TRUE_MSG(runner, expr, msg) \
    ral_test::assert_true_impl(runner, (expr), std::string(#expr) + " [" + (msg) + "]", __FILE__, __LINE__)

#define TEST_ASSERT_LE(runner, a, b) \
    TEST_ASSERT_TRUE_MSG(runner, (a) <= (b), std::to_string(a) + " <= " + std::to_string(b))

#define TEST_ASSERT_GE(runner, a, b) \
    TEST_ASSERT_TRUE_MSG(runner, (a) >= (b), std::to_string(a) + " >= " + std::to_string(b))

#define TEST_ASSERT_GT(runner, a, b) \
    TEST_ASSERT_TRUE_MSG(runner, (a) > (b), std::to_string(a) + " > " + std::to_string(b))

#define TEST_ASSERT_LT(runner, a, b) \
    TEST_ASSERT_TRUE_MSG(runner, (a) < (b), std::to_string(a) + " < " + std::to_string(b))
