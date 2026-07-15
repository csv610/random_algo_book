#pragma once
#include "convex_hull.h"
#include <set>
#include <chrono>

namespace chapter9 {

struct Triangle {
    int v[3];
    Triangle() { v[0] = v[1] = v[2] = -1; }
    Triangle(int a, int b, int c) { v[0] = a; v[1] = b; v[2] = c; }

    bool contains_vertex(int p) const {
        return v[0] == p || v[1] == p || v[2] == p;
    }
};

struct Edge {
    int v[2];
    Edge() { v[0] = v[1] = -1; }
    Edge(int a, int b) {
        v[0] = std::min(a, b);
        v[1] = std::max(a, b);
    }
    bool operator<(const Edge& o) const {
        if (v[0] != o.v[0]) return v[0] < o.v[0];
        return v[1] < o.v[1];
    }
};

// In-circle test: positive if p is inside circumcircle of CCW triangle (a,b,c)
double in_circumcircle(const Point2D& p, const Point2D& a,
                       const Point2D& b, const Point2D& c) {
    double ax = a.x - p.x, ay = a.y - p.y;
    double bx = b.x - p.x, by = b.y - p.y;
    double cx = c.x - p.x, cy = c.y - p.y;

    double det = (ax * ax + ay * ay) * (bx * cy - cx * by)
               - (bx * bx + by * by) * (ax * cy - cx * ay)
               + (cx * cx + cy * cy) * (ax * by - bx * ay);

    // Orient the triangle to be CCW; if CW, negate the result
    double orient = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (orient < 0) det = -det;
    return det;
}

// Randomized incremental Delaunay triangulation with Bowyer-Watson cavity
std::vector<Triangle> delaunay_triangulation(std::vector<Point2D> points) {
    int n = static_cast<int>(points.size());
    if (n < 3) return {};

    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(points.begin(), points.end(), rng);

    // Build super-triangle enclosing all points
    double min_x = points[0].x, max_x = points[0].x;
    double min_y = points[0].y, max_y = points[0].y;
    for (auto& p : points) {
        min_x = std::min(min_x, p.x);
        max_x = std::max(max_x, p.x);
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }

    double dx = max_x - min_x;
    double dy = max_y - min_y;
    double dmax = std::max(dx, dy);
    double midx = (min_x + max_x) / 2.0;
    double midy = (min_y + max_y) / 2.0;

    // Super-triangle: very large, enclosing all points with generous margin
    double margin = 20.0 * dmax;
    points.push_back(Point2D(midx - margin, midy - margin));
    points.push_back(Point2D(midx + margin, midy - margin));
    points.push_back(Point2D(midx, midy + margin));

    std::vector<Triangle> triangles;
    triangles.push_back(Triangle(n, n + 1, n + 2));

    // Insert each point
    for (int i = 0; i < n; i++) {
        Point2D p = points[i];

        // Find triangles whose circumcircle contains p (the "bad" triangles)
        std::vector<int> bad;
        for (int t = 0; t < static_cast<int>(triangles.size()); t++) {
            const Triangle& tri = triangles[t];
            if (in_circumcircle(p, points[tri.v[0]], points[tri.v[1]], points[tri.v[2]]) > 0) {
                bad.push_back(t);
            }
        }

        // Find boundary edges of the polygonal hole
        std::vector<Edge> polygon;
        std::set<int> bad_set(bad.begin(), bad.end());
        for (int tidx : bad) {
            const Triangle& bt = triangles[tidx];
            for (int j = 0; j < 3; j++) {
                Edge e(bt.v[j], bt.v[(j + 1) % 3]);
                bool shared = false;
                for (int tidx2 : bad) {
                    if (tidx2 == tidx) continue;
                    const Triangle& ot = triangles[tidx2];
                    if (ot.contains_vertex(e.v[0]) && ot.contains_vertex(e.v[1])) {
                        shared = true;
                        break;
                    }
                }
                if (!shared) {
                    polygon.push_back(e);
                }
            }
        }

        // Remove bad triangles (reverse order preserves indices)
        std::sort(bad.begin(), bad.end(), std::greater<int>());
        for (int tidx : bad) {
            triangles.erase(triangles.begin() + tidx);
        }

        // Create new triangles from each boundary edge to the new point
        for (auto& e : polygon) {
            triangles.push_back(Triangle(e.v[0], e.v[1], i));
        }
    }

    // Remove triangles that touch the super-triangle
    std::vector<Triangle> result;
    for (auto& t : triangles) {
        if (t.v[0] >= n || t.v[1] >= n || t.v[2] >= n) continue;
        result.push_back(t);
    }

    return result;
}

void demonstrate_delaunay() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<double> dist(-500.0, 500.0);

    auto run_test = [&](int n, const std::string& label) {
        std::vector<Point2D> points(n);
        for (int i = 0; i < n; i++) {
            points[i] = Point2D(dist(rng), dist(rng));
        }

        auto t_start = std::chrono::high_resolution_clock::now();
        auto triangles = delaunay_triangulation(points);
        auto t_end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(t_end - t_start).count();

        std::cout << std::setw(14) << label
                  << "  n=" << std::setw(6) << n
                  << "  triangles=" << std::setw(6) << triangles.size()
                  << "  time=" << std::fixed << std::setprecision(3) << elapsed << " ms\n";

        // For a valid planar triangulation: t = 2n - h - 2 where h = hull vertices
        // t <= 2n - 5
        if (n > 3) {
            int expected_max = 2 * n - 5;
            if (static_cast<int>(triangles.size()) > expected_max) {
                std::cout << "  WARNING: too many triangles\n";
            }
        }
    };

    run_test(50, "n=50");
    run_test(200, "n=200");
    run_test(1000, "n=1000");
    run_test(5000, "n=5000");
}

}  // namespace chapter9
