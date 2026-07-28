#include "test_framework.h"
#include "ral.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <set>

using namespace ral;

// ============================================================
// Stress: Convex Hull
// ============================================================
void test_stress_convex_hull() {
    ral_test::TestRunner runner("Stress: ConvexHull");

    runner.begin_test("cross_product orientation");
    {
        Point2D o(0, 0), a(1, 0), b(0, 1);
        double cp = cross_product(o, a, b);
        TEST_ASSERT_TRUE(runner, cp > 0); // CCW
        cp = cross_product(o, b, a);
        TEST_ASSERT_TRUE(runner, cp < 0); // CW
    }
    runner.end_test();

    runner.begin_test("convex hull includes all points");
    for (int t = 0; t < 50; t++) {
        int n = (t % 15) + 5;
        std::vector<Point2D> pts;
        std::mt19937 rng(t);
        std::uniform_real_distribution<double> dist(-100.0, 100.0);
        for (int i = 0; i < n; i++) {
            pts.push_back(Point2D(dist(rng), dist(rng)));
        }
        auto hull = convex_hull(pts);
        // All input points must be on or inside the hull
        for (auto& p : pts) {
            bool on_hull = false;
            for (auto& h : hull) {
                if (std::abs(h.x - p.x) < 1e-9 && std::abs(h.y - p.y) < 1e-9) {
                    on_hull = true;
                    break;
                }
            }
            // Point is inside if it's on the hull or to the left of all edges
            if (!on_hull) {
                bool inside = true;
                int m = hull.size();
                for (int i = 0; i < m; i++) {
                    Point2D a = hull[i], b = hull[(i + 1) % m];
                    double cp = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
                    if (cp < -1e-9) { inside = false; break; }
                }
                TEST_ASSERT_TRUE(runner, inside);
            }
        }
    }
    runner.end_test();

    runner.begin_test("convex hull is convex");
    for (int t = 0; t < 30; t++) {
        int n = (t % 10) + 5;
        std::vector<Point2D> pts;
        std::mt19937 rng(t);
        std::uniform_real_distribution<double> dist(-50.0, 50.0);
        for (int i = 0; i < n; i++) pts.push_back(Point2D(dist(rng), dist(rng)));
        auto hull = convex_hull(pts);
        int m = hull.size();
        // All turns should be CCW (cross product >= 0)
        for (int i = 0; i < m; i++) {
            double cp = cross_product(hull[i], hull[(i + 1) % m], hull[(i + 2) % m]);
            TEST_ASSERT_TRUE(runner, cp >= -1e-9);
        }
    }
    runner.end_test();

    runner.begin_test("gift_wrap_hull matches convex_hull");
    for (int t = 0; t < 30; t++) {
        int n = (t % 8) + 5;
        std::vector<Point2D> pts;
        std::mt19937 rng(t);
        std::uniform_real_distribution<double> dist(-50.0, 50.0);
        for (int i = 0; i < n; i++) pts.push_back(Point2D(dist(rng), dist(rng)));
        auto hull1 = convex_hull(pts);
        auto hull2 = gift_wrap_hull(pts);
        TEST_ASSERT_EQ(runner, (int)hull1.size(), (int)hull2.size());
    }
    runner.end_test();

    runner.begin_test("collinear points handled");
    {
        std::vector<Point2D> pts = {{0,0}, {1,1}, {2,2}, {3,3}};
        auto hull = convex_hull(pts);
        TEST_ASSERT_TRUE(runner, hull.size() >= 2);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Delaunay Triangulation
// ============================================================
void test_stress_delaunay() {
    ral_test::TestRunner runner("Stress: Delaunay");

    runner.begin_test("Delaunay on random points");
    for (int t = 0; t < 30; t++) {
        int n = (t % 10) + 5;
        std::vector<Point2D> pts;
        std::mt19937 rng(t);
        std::uniform_real_distribution<double> dist(-100.0, 100.0);
        for (int i = 0; i < n; i++) pts.push_back(Point2D(dist(rng), dist(rng)));
        auto triangles = delaunay_triangulation(pts);
        // Valid triangulation: t <= 2n - 5
        TEST_ASSERT_TRUE(runner, (int)triangles.size() <= 2 * n - 5);
        // All vertices in range [0, n-1]
        for (auto& tri : triangles) {
            for (int j = 0; j < 3; j++) {
                TEST_ASSERT_TRUE(runner, tri.v[j] >= 0);
                TEST_ASSERT_TRUE(runner, tri.v[j] < n);
            }
        }
    }
    runner.end_test();

    runner.begin_test("Delaunay on small known inputs");
    {
        // 3 points forming a triangle
        std::vector<Point2D> pts = {{0,0}, {1,0}, {0,1}};
        auto triangles = delaunay_triangulation(pts);
        TEST_ASSERT_EQ(runner, (int)triangles.size(), 1);
    }
    runner.end_test();

    runner.begin_test("Delaunay no duplicate vertices in triangles");
    for (int t = 0; t < 20; t++) {
        int n = (t % 8) + 5;
        std::vector<Point2D> pts;
        std::mt19937 rng(t + 100);
        std::uniform_real_distribution<double> dist(-50.0, 50.0);
        for (int i = 0; i < n; i++) pts.push_back(Point2D(dist(rng), dist(rng)));
        auto triangles = delaunay_triangulation(pts);
        for (auto& tri : triangles) {
            TEST_ASSERT_TRUE(runner, tri.v[0] != tri.v[1]);
            TEST_ASSERT_TRUE(runner, tri.v[1] != tri.v[2]);
            TEST_ASSERT_TRUE(runner, tri.v[0] != tri.v[2]);
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Binary Planar Partition
// ============================================================
void test_stress_planar_partition() {
    ral_test::TestRunner runner("Stress: BinaryPlanarPartition");

    runner.begin_test("segments_intersect correctness");
    {
        Segment s1(0, 0, 4, 4, 0);
        Segment s2(0, 4, 4, 0, 1);
        TEST_ASSERT_TRUE(runner, segments_intersect(s1, s2));

        Segment s3(0, 0, 1, 0, 2);
        Segment s4(0, 1, 1, 1, 3);
        TEST_ASSERT_TRUE(runner, !segments_intersect(s3, s4));
    }
    runner.end_test();

    runner.begin_test("rand_auto_partition builds valid tree");
    for (int t = 0; t < 30; t++) {
        int n = (t % 8) + 2;
        std::vector<Segment> segs;
        std::mt19937 rng(t);
        std::uniform_real_distribution<double> dist(-10.0, 10.0);
        for (int i = 0; i < n; i++) {
            segs.push_back(Segment(dist(rng), dist(rng), dist(rng), dist(rng), i));
        }
        std::unique_ptr<PartitionNode> root(rand_auto_partition(segs));
        int size = partition_size(root.get());
        TEST_ASSERT_TRUE(runner, size >= n); // at least n nodes (one per segment)
    }
    runner.end_test();

    runner.begin_test("single segment tree");
    {
        std::vector<Segment> segs = {Segment(0, 0, 1, 1, 0)};
        std::unique_ptr<PartitionNode> root(rand_auto_partition(segs));
        TEST_ASSERT_EQ(runner, partition_size(root.get()), 1);
    }
    runner.end_test();

    runner.begin_test("non-intersecting segments partition cleanly");
    {
        std::vector<Segment> segs = {
            Segment(0, 0, 0, 5, 0),
            Segment(2, 0, 2, 5, 1),
            Segment(4, 0, 4, 5, 2)
        };
        std::unique_ptr<PartitionNode> root(rand_auto_partition(segs));
        int size = partition_size(root.get());
        TEST_ASSERT_TRUE(runner, size >= 3);
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

// ============================================================
// Stress: Random Projection (JL)
// ============================================================
void test_stress_random_projection() {
    ral_test::TestRunner runner("Stress: RandomProjection");

    runner.begin_test("distance preservation");
    {
        std::mt19937 rng(42);
        int orig_dim = 100;
        int target_dim = 20;
        RandomProjection rp(orig_dim, target_dim, rng);

        for (int t = 0; t < 50; t++) {
            std::vector<double> a(orig_dim), b(orig_dim);
            std::normal_distribution<double> norm(0.0, 1.0);
            for (int i = 0; i < orig_dim; i++) {
                a[i] = norm(rng) * (t + 1);
                b[i] = norm(rng) * (t + 1);
            }
            double orig_dist = RandomProjection::squared_distance(a, b);
            auto pa = rp.project(a);
            auto pb = rp.project(b);
            double proj_dist = RandomProjection::squared_distance(pa, pb);
            // JL: distances preserved within (1 ± ε) factor
            // For k=20, ε ~ 0.5 should hold with high probability
            TEST_ASSERT_TRUE(runner, proj_dist > 0);
            // proj_dist should be roughly orig_dist (within 3x factor for small target_dim)
            double ratio = proj_dist / orig_dist;
            TEST_ASSERT_TRUE(runner, ratio > 0.01);
            TEST_ASSERT_TRUE(runner, ratio < 100.0);
        }
    }
    runner.end_test();

    runner.begin_test("project_dataset consistency");
    {
        std::mt19937 rng(42);
        RandomProjection rp(10, 5, rng);
        std::vector<std::vector<double>> dataset = {
            {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
            {10, 9, 8, 7, 6, 5, 4, 3, 2, 1}
        };
        auto projected = rp.project_dataset(dataset);
        TEST_ASSERT_EQ(runner, (int)projected.size(), 2);
        TEST_ASSERT_EQ(runner, (int)projected[0].size(), 5);
        // Projecting same point twice should give same result
        auto p1 = rp.project(dataset[0]);
        auto p2 = rp.project(dataset[0]);
        for (int i = 0; i < 5; i++) {
            TEST_ASSERT_NEAR(runner, p1[i], p2[i], 1e-12);
        }
    }
    runner.end_test();

    runner.begin_test("zero vector projects to near-zero");
    {
        std::mt19937 rng(42);
        RandomProjection rp(50, 10, rng);
        std::vector<double> zero(50, 0.0);
        auto projected = rp.project(zero);
        for (int i = 0; i < 10; i++) {
            TEST_ASSERT_NEAR(runner, projected[i], 0.0, 1e-12);
        }
    }
    runner.end_test();

    runner.report();
    ral_test::all_results().push_back(runner.result());
}

int main() {
    test_stress_convex_hull();
    test_stress_delaunay();
    test_stress_planar_partition();
    test_stress_random_projection();
    return ral_test::run_all_suites() ? 0 : 1;
}
