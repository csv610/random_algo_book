#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cassert>
#include <cmath>

namespace chapter10 {

struct Point2D {
    double x, y;
    Point2D() : x(0), y(0) {}
    Point2D(double x, double y) : x(x), y(y) {}

    bool operator==(const Point2D& o) const {
        return x == o.x && y == o.y;
    }
    bool operator<(const Point2D& o) const {
        return x < o.x || (x == o.x && y < o.y);
    }
};

double cross_product(const Point2D& O, const Point2D& A, const Point2D& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

double dist2(const Point2D& a, const Point2D& b) {
    double dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// Jarvis march (gift wrapping) -- O(nh) where h = hull size
std::vector<Point2D> gift_wrap_hull(std::vector<Point2D> points) {
    int n = static_cast<int>(points.size());
    if (n < 3) return points;

    std::vector<Point2D> hull;
    int leftmost = 0;
    for (int i = 1; i < n; i++) {
        if (points[i].x < points[leftmost].x ||
            (points[i].x == points[leftmost].x && points[i].y < points[leftmost].y)) {
            leftmost = i;
        }
    }

    int p = leftmost;
    do {
        hull.push_back(points[p]);
        int q = (p + 1) % n;
        for (int r = 0; r < n; r++) {
            double cp = cross_product(points[p], points[q], points[r]);
            if (cp < 0 || (cp == 0 && dist2(points[p], points[r]) > dist2(points[p], points[q]))) {
                q = r;
            }
        }
        p = q;
    } while (p != leftmost);

    return hull;
}

// Randomized incremental convex hull -- expected O(n log n)
// Maintains upper and lower chains separately
std::vector<Point2D> convex_hull(std::vector<Point2D> points) {
    int n = static_cast<int>(points.size());
    if (n < 3) return points;

    std::random_device rd;
    std::mt19937 rng(rd());

    // Shuffle the points
    std::shuffle(points.begin(), points.end(), rng);

    // Sort by x to initialize with two leftmost points
    std::sort(points.begin(), points.end(), [](const Point2D& a, const Point2D& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });

    // Remove duplicates
    auto last = std::unique(points.begin(), points.end(), [](const Point2D& a, const Point2D& b) {
        return a.x == b.x && a.y == b.y;
    });
    points.erase(last, points.end());
    n = static_cast<int>(points.size());
    if (n < 3) return points;

    // Build upper and lower hulls incrementally
    std::vector<Point2D> lower;
    std::vector<Point2D> upper;

    lower.push_back(points[0]);
    lower.push_back(points[1]);
    upper.push_back(points[0]);
    upper.push_back(points[1]);

    for (int i = 2; i < n; i++) {
        // Add to lower chain
        lower.push_back(points[i]);
        while (lower.size() > 2) {
            int sz = static_cast<int>(lower.size());
            double cp = cross_product(lower[sz - 3], lower[sz - 2], lower[sz - 1]);
            if (cp <= 0) {
                // middle point is not part of hull; remove it
                lower.erase(lower.end() - 2);
            } else {
                break;
            }
        }

        // Add to upper chain
        upper.push_back(points[i]);
        while (upper.size() > 2) {
            int sz = static_cast<int>(upper.size());
            double cp = cross_product(upper[sz - 3], upper[sz - 2], upper[sz - 1]);
            if (cp >= 0) {
                // middle point is not part of hull; remove it
                upper.erase(upper.end() - 2);
            } else {
                break;
            }
        }
    }

    // Combine: lower chain + upper chain (excluding endpoints to avoid duplication)
    std::vector<Point2D> hull;
    for (int i = 0; i < static_cast<int>(lower.size()); i++) {
        hull.push_back(lower[i]);
    }
    for (int i = static_cast<int>(upper.size()) - 2; i >= 1; i--) {
        hull.push_back(upper[i]);
    }

    return hull;
}

void demonstrate_convex_hull() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<double> dist(-1000.0, 1000.0);

    auto run_test = [&](int n, const std::string& label) {
        std::vector<Point2D> points(n);
        for (int i = 0; i < n; i++) {
            points[i] = Point2D(dist(rng), dist(rng));
        }

        auto t_start = std::chrono::high_resolution_clock::now();
        auto hull = convex_hull(points);
        auto t_end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(t_end - t_start).count();

        std::cout << std::setw(14) << label
                  << "  n=" << std::setw(7) << n
                  << "  hull=" << std::setw(5) << hull.size()
                  << "  time=" << std::fixed << std::setprecision(3) << elapsed << " ms\n";

        // Verify hull is counterclockwise
        int h = static_cast<int>(hull.size());
        for (int i = 0; i < h; i++) {
            double cp = cross_product(hull[i], hull[(i + 1) % h], hull[(i + 2) % h]);
            if (cp < -1e-9) {
                std::cout << "  WARNING: hull may not be counterclockwise at vertex " << i << "\n";
            }
        }
    };

    run_test(100, "n=100");
    run_test(1000, "n=1000");
    run_test(10000, "n=10000");
    run_test(100000, "n=100000");
}

}  // namespace chapter10
