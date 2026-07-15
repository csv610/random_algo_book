#pragma once

#include <vector>
#include <random>
#include <print>
#include <format>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cassert>

namespace randalgo {

// ---------------------------------------------------------------------------
// Polytope in H-representation:  { x \in R^n : Ax <= b }
// ---------------------------------------------------------------------------

struct Polytope {
    int dimension;
    std::vector<std::vector<double>> A;  // constraints: A[i] is a row
    std::vector<double> b;               // right-hand side

    Polytope() : dimension(0) {}

    // Check if point satisfies all constraints
    bool contains(const std::vector<double>& x) const {
        for (int i = 0; i < static_cast<int>(A.size()); ++i) {
            double dot = 0.0;
            for (int j = 0; j < dimension; ++j)
                dot += A[i][j] * x[j];
            if (dot > b[i] + 1e-12) return false;
        }
        return true;
    }

    // Add constraint a.x <= b_val
    void add_constraint(std::vector<double> a_row, double b_val) {
        A.push_back(std::move(a_row));
        b.push_back(b_val);
    }
};

// ---------------------------------------------------------------------------
// Simplex in R^n defined by n+1 vertices (V-representation)
// ---------------------------------------------------------------------------

struct Simplex {
    int dimension;
    std::vector<std::vector<double>> vertices;  // n+1 vertices

    Simplex() : dimension(0) {}

    // Convert to H-representation (polytope) using the fact that
    // a simplex with vertices v_0,...,v_n has faces defined by the
    // hyperplanes through each subset of n vertices.
    Polytope to_polytope() const {
        int nv = dimension + 1;
        Polytope p;
        p.dimension = dimension;

        // For each face (omit vertex j), find the hyperplane through
        // the remaining n vertices, oriented inward.
        for (int j = 0; j < nv; ++j) {
            // Build system: solve for hyperplane coefficients through
            // vertices 0..j-1, j+1..nv-1
            // Use n points to define the hyperplane n.x = d
            std::vector<std::vector<double>> pts;
            for (int i = 0; i < nv; ++i) {
                if (i == j) continue;
                pts.push_back(vertices[i]);
            }

            // Fit hyperplane: use first vertex as reference
            // For simplicity in low dimensions, use the general approach:
            // Solve the system [v_0-v_j, v_1-v_j, ...]^T . n = 0
            // where n is the outward normal, then flip sign.

            if (dimension == 2) {
                // Edge opposite vertex j: line through pts[0] and pts[1]
                double dx = pts[1][0] - pts[0][0];
                double dy = pts[1][1] - pts[0][1];
                // Normal: (dy, -dx) or (-dy, dx)
                double nx = dy;
                double ny = -dx;
                // Orient so that vertex j is on the inside
                double check = nx * (vertices[j][0] - pts[0][0])
                             + ny * (vertices[j][1] - pts[0][1]);
                if (check < 0) { nx = -nx; ny = -ny; }
                double d = nx * pts[0][0] + ny * pts[0][1];
                p.add_constraint({nx, ny}, d);
            } else if (dimension == 3) {
                // Plane through pts[0], pts[1], pts[2]
                double ax = pts[1][0]-pts[0][0], ay = pts[1][1]-pts[0][1],
                       az = pts[1][2]-pts[0][2];
                double bx = pts[2][0]-pts[0][0], by = pts[2][1]-pts[0][1],
                       bz = pts[2][2]-pts[0][2];
                double nx = ay*bz - az*by;
                double ny = az*bx - ax*bz;
                double nz = ax*by - ay*bx;
                double check = nx*(vertices[j][0]-pts[0][0])
                             + ny*(vertices[j][1]-pts[0][1])
                             + nz*(vertices[j][2]-pts[0][2]);
                if (check < 0) { nx=-nx; ny=-ny; nz=-nz; }
                double d = nx*pts[0][0]+ny*pts[0][1]+nz*pts[0][2];
                p.add_constraint({nx, ny, nz}, d);
            } else {
                // General: use pseudoinverse approach
                // Build matrix M where rows are (v_i - v_j) for i != j
                int m = dimension;  // n equations, n unknowns
                std::vector<std::vector<double>> M(m, std::vector<double>(dimension));
                std::vector<double> rhs(m, 0.0);
                for (int r = 0; r < m; ++r) {
                    for (int c = 0; c < dimension; ++c) {
                        M[r][c] = pts[r][c] - vertices[j][c];
                    }
                }
                // Solve M . normal = 0 with least-squares using first row as pivot
                // For simplicity: use row reduction
                std::vector<double> normal(dimension, 0.0);
                // Set last variable to 1 and solve
                normal[dimension - 1] = 1.0;
                for (int r = 0; r < std::min(m, dimension - 1); ++r) {
                    double pivot = M[r][r];
                    if (std::abs(pivot) < 1e-15) continue;
                    double scale = 1.0 / pivot;
                    normal[r] = -M[r][dimension - 1] * scale;
                    for (int rr = r + 1; rr < m; ++rr) {
                        double factor = M[rr][r] * scale;
                        for (int c = r; c < dimension; ++c)
                            M[rr][c] -= factor * M[r][c];
                    }
                }
                double check = 0.0;
                for (int c = 0; c < dimension; ++c)
                    check += normal[c] * (vertices[j][c] - pts[0][c]);
                if (check < 0) {
                    for (auto& v : normal) v = -v;
                }
                double d = 0.0;
                for (int c = 0; c < dimension; ++c)
                    d += normal[c] * pts[0][c];
                p.add_constraint(std::move(normal), d);
            }
        }
        return p;
    }
};

// ---------------------------------------------------------------------------
// Exact volumes for reference
// ---------------------------------------------------------------------------

double unit_cube_volume() { return 1.0; }

double unit_ball_volume(int d) {
    // vol(B^d) = pi^(d/2) / Gamma(d/2 + 1)
    double half = d / 2.0;
    return std::exp(half * std::log(M_PI) - std::lgamma(half + 1.0));
}

double simplex_volume(int d) {
    // Regular simplex with vertices at origin and unit vectors
    // Volume = 1/d!
    return 1.0 / std::tgamma(d + 1.0);
}

// ---------------------------------------------------------------------------
// Random utilities
// ---------------------------------------------------------------------------

// Unit vector in a random direction in R^d
std::vector<double> random_direction(int d, std::mt19937& rng) {
    std::normal_distribution<double> norm(0.0, 1.0);
    std::vector<double> dir(d);
    double len = 0.0;
    for (int i = 0; i < d; ++i) {
        dir[i] = norm(rng);
        len += dir[i] * dir[i];
    }
    len = std::sqrt(len);
    for (auto& v : dir) v /= len;
    return dir;
}

// Uniform random point in a ball of given radius (rejection-free via
// Gaussian trick: normalize a Gaussian vector, multiply by U^{1/d})
std::vector<double> sample_point_in_ball(int d, double radius,
                                          std::mt19937& rng) {
    std::normal_distribution<double> norm(0.0, 1.0);
    std::uniform_real_distribution<double> unif(0.0, 1.0);

    // Generate Gaussian point and normalize to unit sphere
    std::vector<double> pt(d);
    double len = 0.0;
    for (int i = 0; i < d; ++i) {
        pt[i] = norm(rng);
        len += pt[i] * pt[i];
    }
    len = std::sqrt(len);

    // Scale by U^{1/d} for uniform distribution in ball
    double u = std::pow(unif(rng), 1.0 / d);
    double r = radius * u;
    for (auto& v : pt) v = v / len * r;
    return pt;
}

// Dot product
double dot(const std::vector<double>& a, const std::vector<double>& b) {
    double s = 0.0;
    for (int i = 0; i < static_cast<int>(a.size()); ++i)
        s += a[i] * b[i];
    return s;
}

// ---------------------------------------------------------------------------
// Hit-and-run sampler: starts at a point inside the polytope, takes
// a random direction, finds the feasible interval, and samples uniformly.
// ---------------------------------------------------------------------------

// Find the feasible interval [t_min, t_max] along direction d from point x
// in the polytope Ax <= b.
std::pair<double, double> feasible_interval(const Polytope& p,
                                            const std::vector<double>& x,
                                            const std::vector<double>& d) {
    double t_min = -1e18, t_max = 1e18;
    for (int i = 0; i < static_cast<int>(p.A.size()); ++i) {
        double Ad = dot(p.A[i], d);
        double Ax = dot(p.A[i], x);
        double slack = p.b[i] - Ax;
        if (std::abs(Ad) < 1e-15) {
            // Constraint is parallel; if Ax > b, infeasible (shouldn't happen)
            continue;
        }
        double t = slack / Ad;
        if (Ad > 0) {
            t_max = std::min(t_max, t);
        } else {
            t_min = std::max(t_min, t);
        }
    }
    return {t_min, t_max};
}

// One hit-and-run step
std::vector<double> hit_and_run_step(const Polytope& p,
                                      const std::vector<double>& x,
                                      std::mt19937& rng) {
    int d = p.dimension;
    auto dir = random_direction(d, rng);
    auto [t_lo, t_hi] = feasible_interval(p, x, dir);

    std::uniform_real_distribution<double> unif(t_lo, t_hi);
    double t = unif(rng);

    std::vector<double> new_x(d);
    for (int i = 0; i < d; ++i)
        new_x[i] = x[i] + t * dir[i];
    return new_x;
}

// ---------------------------------------------------------------------------
// Approximate bounding sphere using random directions
// For each random direction, find the farthest point of the polytope
// in that direction via a simple linear scan / linear program.
// ---------------------------------------------------------------------------

std::vector<double> bounding_sphere(const Polytope& p,
                                     std::mt19937& rng) {
    int d = p.dimension;

    // Find an interior point analytically: use the centroid of the
    // bounding box, then check feasibility.
    std::vector<double> center(d, 0.0);
    for (int j = 0; j < d; ++j) {
        double lo = -1e18, hi = 1e18;
        for (int i = 0; i < static_cast<int>(p.A.size()); ++i) {
            double a_ij = p.A[i][j];
            if (std::abs(a_ij) < 1e-15) continue;
            double bound = p.b[i] / a_ij;
            if (a_ij > 0) hi = std::min(hi, bound);
            else           lo = std::max(lo, bound);
        }
        center[j] = (lo + hi) / 2.0;
    }

    // If center is not feasible (shouldn't happen for well-formed polytopes),
    // do a random search.
    if (!p.contains(center)) {
        std::uniform_real_distribution<double> coord(-10.0, 10.0);
        for (int attempt = 0; attempt < 10000; ++attempt) {
            for (int j = 0; j < d; ++j) center[j] = coord(rng);
            if (p.contains(center)) break;
        }
    }

    return center;
}

// ---------------------------------------------------------------------------
// Volume estimation via rejection sampling (simple baseline)
// ---------------------------------------------------------------------------

double volume_rejection_sampling(const Polytope& p,
                                  const std::vector<double>& center,
                                  double radius,
                                  int num_samples,
                                  std::mt19937& rng) {
    int d = p.dimension;
    double ball_vol = unit_ball_volume(d) * std::pow(radius, d);
    int hits = 0;
    for (int i = 0; i < num_samples; ++i) {
        auto pt = sample_point_in_ball(d, radius, rng);
        // Translate to center
        for (int j = 0; j < d; ++pt[j] += center[j], ++j);
        if (p.contains(pt))
            ++hits;
    }
    return ball_vol * static_cast<double>(hits) / static_cast<double>(num_samples);
}

// ---------------------------------------------------------------------------
// Volume estimation via Dyer-Frieze-Kannan-style sequential shrinking
//
// High-level idea:
//   1. Start with a large ball B_0 containing the polytope K.
//   2. At each step t, we have a ball B_t.
//   3. Sample points from B_t using hit-and-run.
//   4. Construct a shrunken ball B_{t+1} = B_t & H where H is a random
//      half-space through the center of B_t that contains most of K.
//   5. The ratio vol(B_{t+1})/vol(B_t) is estimated from the sampling.
//   6. Final estimate = vol(B_0) x PROD ratios.
//
// For simplicity, we use a variant where we repeatedly shrink a ball
// by a factor estimated from random samples.
// ---------------------------------------------------------------------------

double volume_convex_body_approx(const Polytope& p, std::mt19937& rng,
                                  int num_phases = 50,
                                  int samples_per_phase = 500) {
    int d = p.dimension;

    // Find bounding sphere
    auto center = bounding_sphere(p, rng);
    double max_r = 0.0;
    for (int i = 0; i < 200; ++i) {
        auto dir = random_direction(d, rng);
        auto [lo, hi] = feasible_interval(p, center, dir);
        max_r = std::max(max_r, std::max(std::abs(lo), std::abs(hi)));
    }
    double radius = max_r * 1.1;  // small margin

    double vol = unit_ball_volume(d) * std::pow(radius, d);

    // Start hit-and-run from center
    std::vector<double> x = center;
    if (!p.contains(x)) {
        // Fallback: search for an interior point
        std::uniform_real_distribution<double> coord(-1.0, 1.0);
        for (int attempt = 0; attempt < 10000; ++attempt) {
            for (int j = 0; j < d; ++j) x[j] = coord(rng);
            if (p.contains(x)) break;
        }
    }

    // Burn-in
    for (int i = 0; i < 200; ++i)
        x = hit_and_run_step(p, x, rng);

    // Sequential shrinking: at each phase, estimate the fraction of the
    // current ball that lies inside K, and shrink accordingly.
    std::vector<double> center_ball = x;

    double shrink_ratio = 1.0;
    for (int phase = 0; phase < num_phases; ++phase) {
        // Sample points via hit-and-run and check feasibility
        int inside = 0;
        for (int s = 0; s < samples_per_phase; ++s) {
            x = hit_and_run_step(p, x, rng);
            // Check if x is within current "effective radius"
            double dist_sq = 0.0;
            for (int j = 0; j < d; ++j)
                dist_sq += (x[j] - center_ball[j]) * (x[j] - center_ball[j]);
            if (std::sqrt(dist_sq) <= radius)
                ++inside;
        }

        double fraction =
            static_cast<double>(inside) / static_cast<double>(samples_per_phase);

        // Shrink: reduce radius by estimated fraction
        double new_radius = radius * std::pow(fraction, 1.0 / d);
        if (new_radius < 1e-15) new_radius = 1e-15;

        double ratio = std::pow(new_radius / radius, d);
        shrink_ratio *= ratio;
        radius = new_radius;

        if (phase < 5 || phase % 10 == 0) {
            std::println("    Phase {:2d}: radius = {:.6f}  fraction_inside = {:.4f}  "
                         "cumul_ratio = {:.6f}",
                         phase, radius, fraction, shrink_ratio);
        }
    }

    return vol * shrink_ratio;
}

// ---------------------------------------------------------------------------
// Simpler educational volume estimator: Monte Carlo with bounding box
// Works well for low dimensions. Returns (estimate, bounding_box_volume).
// ---------------------------------------------------------------------------

struct VolumeResult {
    double estimate;
    double bounding_volume;
    int samples;
};

VolumeResult volume_monte_carlo(const Polytope& p, int num_samples,
                                 std::mt19937& rng) {
    int d = p.dimension;

    // Analytic bounding box: for each dimension j, find lo[j] and hi[j]
    // from the constraints. For constraint A[i].x <= b[i], setting all
    // other variables to 0 gives A[i][j]*x_j <= b[i].
    std::vector<double> lo(d, -1e18), hi(d, 1e18);
    for (int j = 0; j < d; ++j) {
        for (int i = 0; i < static_cast<int>(p.A.size()); ++i) {
            double a_ij = p.A[i][j];
            if (std::abs(a_ij) < 1e-15) continue;
            double bound = p.b[i] / a_ij;
            if (a_ij > 0) {
                hi[j] = std::min(hi[j], bound);
            } else {
                lo[j] = std::max(lo[j], bound);
            }
        }
    }

    // Expand bounds slightly for numerical safety
    for (int j = 0; j < d; ++j) {
        double span = hi[j] - lo[j];
        lo[j] -= span * 0.05;
        hi[j] += span * 0.05;
    }

    double box_vol = 1.0;
    for (int j = 0; j < d; ++j)
        box_vol *= (hi[j] - lo[j]);

    // Rejection sampling
    int hits = 0;
    for (int i = 0; i < num_samples; ++i) {
        std::vector<double> pt(d);
        for (int j = 0; j < d; ++j) {
            std::uniform_real_distribution<double> coord(lo[j], hi[j]);
            pt[j] = coord(rng);
        }
        if (p.contains(pt))
            ++hits;
    }

    return {box_vol * static_cast<double>(hits) / static_cast<double>(num_samples),
            box_vol, num_samples};
}

// ---------------------------------------------------------------------------
// Build standard convex bodies
// ---------------------------------------------------------------------------

// Unit cube [0,1]^d
Polytope unit_cube(int d) {
    Polytope p;
    p.dimension = d;
    for (int i = 0; i < d; ++i) {
        std::vector<double> pos(d, 0.0), neg(d, 0.0);
        pos[i] = 1.0;
        neg[i] = -1.0;
        p.add_constraint(pos, 1.0);   // x_i <= 1
        p.add_constraint(neg, 0.0);   // -x_i <= 0  ->  x_i >= 0
    }
    return p;
}

// Cross-polytope (unit ball in L1 norm): |x_1| + ... + |x_d| <= 1
// Implemented as 2^d linear constraints
Polytope unit_cross_polytope(int d) {
    Polytope p;
    p.dimension = d;
    // For each choice of signs (+/-1,...,+/-1), constraint: Sigma s_i x_i <= 1
    int num_faces = 1 << d;
    for (int mask = 0; mask < num_faces; ++mask) {
        std::vector<double> row(d);
        for (int i = 0; i < d; ++i)
            row[i] = ((mask >> i) & 1) ? 1.0 : -1.0;
        p.add_constraint(std::move(row), 1.0);
    }
    return p;
}

// Simplex: convex hull of origin and unit vectors e_1, ..., e_d
// H-representation: x_i >= 0 for all i, and x_1 + ... + x_d <= 1
Polytope standard_simplex(int d) {
    Polytope p;
    p.dimension = d;
    // x_i >= 0
    for (int i = 0; i < d; ++i) {
        std::vector<double> row(d, 0.0);
        row[i] = -1.0;
        p.add_constraint(row, 0.0);
    }
    // Sum <= 1
    std::vector<double> ones(d, 1.0);
    p.add_constraint(std::move(ones), 1.0);
    return p;
}

// ---------------------------------------------------------------------------
// Print helper
// ---------------------------------------------------------------------------

void print_polytope_info(const Polytope& p, const std::string& name) {
    std::println("  {}: dimension = {}, constraints = {}",
                 name, p.dimension, p.A.size());
}

// ---------------------------------------------------------------------------
// Demonstration
// ---------------------------------------------------------------------------

void demonstrate_volume() {
    std::mt19937 rng(
        static_cast<unsigned>(std::chrono::steady_clock::now()
                                  .time_since_epoch()
                                  .count()));

    // ---------------------------------------------------------------
    // Part 1: Unit cube -- exact volume = 1.0
    // ---------------------------------------------------------------
    std::println("=== Part 1: Volume of the Unit Cube ===\n");

    for (int d : {2, 3, 5, 10}) {
        auto cube = unit_cube(d);
        double exact = unit_cube_volume();
        auto [est, bv, n] = volume_monte_carlo(cube, 50000, rng);
        std::println("  dim={:2d}:  exact = {:.6f}  MC estimate = {:.6f}  "
                     "error = {:.6f}",
                     d, exact, est, std::abs(est - exact));
    }

    // ---------------------------------------------------------------
    // Part 2: Unit ball (cross-polytope approximation)
    // ---------------------------------------------------------------
    std::println("\n=== Part 2: Volume of the L1 Unit Ball (Cross-Polytope) ===\n");
    std::println("  The L1 ball: |x_1| + ... + |x_d| <= 1  has volume 2^d/d!\n");

    for (int d : {2, 3, 4, 5}) {
        auto ball = unit_cross_polytope(d);
        double exact = std::ldexp(1.0, d) / std::tgamma(d + 1.0);  // 2^d / d!
        auto [est, bv, n] = volume_monte_carlo(ball, 50000, rng);
        std::println("  dim={:2d}:  exact = {:.6f}  MC estimate = {:.6f}  "
                     "error = {:.6f}  (2^{:d}/{}! = {:.4f})",
                     d, exact, est, std::abs(est - exact), d, d, exact);
    }

    // ---------------------------------------------------------------
    // Part 3: Standard simplex
    // ---------------------------------------------------------------
    std::println("\n=== Part 3: Volume of the Standard Simplex ===\n");

    for (int d : {2, 3, 4, 5}) {
        auto simp = standard_simplex(d);
        double exact = simplex_volume(d);
        auto [est, bv, n] = volume_monte_carlo(simp, 50000, rng);
        std::println("  dim={:2d}:  exact = {:.6f}  MC estimate = {:.6f}  "
                     "error = {:.6f}  (1/{}! = 1/{})",
                     d, exact, est, std::abs(est - exact), d,
                     static_cast<int>(std::tgamma(d + 1)));
    }

    // ---------------------------------------------------------------
    // Part 4: Step-by-step shrinking for low dimensions
    // ---------------------------------------------------------------
    std::println("\n=== Part 4: Sequential Shrinking Process (dim=2) ===\n");

    {
        auto simp = standard_simplex(2);
        print_polytope_info(simp, "2D Simplex");
        std::println("  Exact volume: {:.6f}", simplex_volume(2));
        std::println();

        // Show hit-and-run walk and volume estimation
        std::vector<double> start = {0.2, 0.2};  // interior point
        std::println("  Hit-and-run walk (first 10 steps):");
        auto x = start;
        for (int i = 0; i < 10; ++i) {
            x = hit_and_run_step(simp, x, rng);
            std::println("    step {:2d}: ({:.4f}, {:.4f})  inside={}",
                         i + 1, x[0], x[1], simp.contains(x));
        }

        std::println("\n  Volume estimation via Monte Carlo (dim=2):");
        for (int samples : {100, 500, 1000, 5000, 10000}) {
            auto [est, bv, n] = volume_monte_carlo(simp, samples, rng);
            std::println("    {:6d} samples: est = {:.6f}  err = {:.6f}",
                         samples, est, std::abs(est - 0.5));
        }
    }

    // ---------------------------------------------------------------
    // Part 5: Step-by-step for dim=3
    // ---------------------------------------------------------------
    std::println("\n=== Part 5: Step-by-Step Process (dim=3) ===\n");

    {
        auto cube = unit_cube(3);
        print_polytope_info(cube, "3D Unit Cube");
        std::println("  Exact volume: {:.6f}", unit_cube_volume());

        // Hit-and-run walk
        std::vector<double> x = {0.3, 0.3, 0.3};
        std::println("\n  Hit-and-run walk (first 15 steps):");
        for (int i = 0; i < 15; ++i) {
            x = hit_and_run_step(cube, x, rng);
            std::println("    step {:2d}: ({:.4f}, {:.4f}, {:.4f})  inside={}",
                         i + 1, x[0], x[1], x[2], cube.contains(x));
        }

        // Monte Carlo volume
        std::println("\n  Volume estimation via Monte Carlo (dim=3):");
        for (int samples : {1000, 5000, 10000, 50000}) {
            auto [est, bv, n] = volume_monte_carlo(cube, samples, rng);
            std::println("    {:6d} samples: est = {:.6f}  err = {:.6f}",
                         samples, est, std::abs(est - 1.0));
        }
    }

    // ---------------------------------------------------------------
    // Part 6: Convergence study across dimensions
    // ---------------------------------------------------------------
    std::println("\n=== Part 6: Convergence Study Across Dimensions ===\n");

    for (int d : {2, 3, 4}) {
        auto simp = standard_simplex(d);
        double exact = simplex_volume(d);
        std::println("  dim={} simplex (exact={:.6f}):", d, exact);

        for (int samples : {1000, 5000, 10000}) {
            // Average over 5 runs
            double sum = 0.0;
            for (int run = 0; run < 5; ++run) {
                auto [est, bv, n] = volume_monte_carlo(simp, samples, rng);
                sum += est;
            }
            double avg = sum / 5.0;
            double rel_err = 100.0 * std::abs(avg - exact) / exact;
            std::println("    {:6d} samples (5 runs avg): est = {:.6f}  "
                         "rel_error = {:.2f}%",
                         samples, avg, rel_err);
        }
        std::println();
    }

    // ---------------------------------------------------------------
    // Part 7: Volume comparison table
    // ---------------------------------------------------------------
    std::println("=== Part 7: Volume Comparison Table ===\n");
    std::println("  MC estimates (30k samples) vs exact analytical volumes.\n");
    std::println("  {:>4s}  {:>16s}  {:>16s}  {:>16s}",
                 "dim", "Cube [0,1]^d", "Simplex (1/d!)",
                 "L1 Ball (2^d/d!)");
    std::println("  {:>4s}  {:>16s}  {:>16s}  {:>16s}",
                 "---", "--------", "------------",
                 "---------------");

    for (int d : {2, 3, 4, 5, 6}) {
        auto cube = unit_cube(d);
        auto simp = standard_simplex(d);
        auto cross = unit_cross_polytope(d);

        auto est_c = volume_monte_carlo(cube, 30000, rng).estimate;
        auto est_s = volume_monte_carlo(simp, 30000, rng).estimate;
        auto est_x = volume_monte_carlo(cross, 30000, rng).estimate;

        // Exact cross-polytope volume: 2^d / d!
        double exact_cross = std::ldexp(1.0, d) / std::tgamma(d + 1.0);

        std::println("  {:4d}  {:7.4f} / {:<5.1f}  {:7.4f} / {:<7.4f}  {:7.4f} / {:<7.4f}",
                     d, est_c, 1.0,
                     est_s, simplex_volume(d),
                     est_x, exact_cross);
    }

    std::println("\n=== Volume Estimation Complete ===\n");
}

} // namespace randalgo
