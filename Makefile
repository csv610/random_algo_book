CXX = g++
CXXFLAGS = -std=c++23 -O1 -Wall -Wextra -Wpedantic

BUILDDIR = build

# Chapter 2 is omitted as it only contains inline code listings in the book
CHAPTERS = 1 3 4 5 6 7 8 9 10 11 12 13 14 15

# Auto-discover all example_*.cpp files
EXAMPLE_SRCS = $(wildcard examples/example_*.cpp)
EXAMPLE_BINS = $(patsubst examples/%.cpp,$(BUILDDIR)/%,$(EXAMPLE_SRCS))

.PHONY: all clean run examples run-examples test coverage

all: build/chapter1 build/chapter3 build/chapter4 build/chapter5 build/chapter6 build/chapter7 build/chapter8 build/chapter9 build/chapter10 build/chapter11 build/chapter12 build/chapter13 build/chapter14 build/chapter15

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/chapter1: src/chapter1/main.cpp src/chapter1/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter1 -o $@ $<

$(BUILDDIR)/chapter3: src/chapter3/main.cpp src/chapter3/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter3 -o $@ $<

$(BUILDDIR)/chapter4: src/chapter4/main.cpp src/chapter4/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter4 -o $@ $<

$(BUILDDIR)/chapter5: src/chapter5/main.cpp src/chapter5/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter5 -o $@ $<

$(BUILDDIR)/chapter6: src/chapter6/main.cpp src/chapter6/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter6 -o $@ $<

$(BUILDDIR)/chapter7: src/chapter7/main.cpp src/chapter7/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter7 -o $@ $<

$(BUILDDIR)/chapter8: src/chapter8/main.cpp src/chapter8/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter8 -o $@ $<

$(BUILDDIR)/chapter9: src/chapter9/main.cpp src/chapter9/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter9 -o $@ $<

$(BUILDDIR)/chapter10: src/chapter10/main.cpp src/chapter10/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter10 -o $@ $<

$(BUILDDIR)/chapter11: src/chapter11/main.cpp src/chapter11/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter11 -o $@ $<

$(BUILDDIR)/chapter12: src/chapter12/main.cpp src/chapter12/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter12 -o $@ $<

$(BUILDDIR)/chapter13: src/chapter13/main.cpp src/chapter13/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter13 -o $@ $<

# Chapter 14 (Online Algorithms) depends on random_utils.h in Chapter 11
$(BUILDDIR)/chapter14: src/chapter14/main.cpp src/chapter14/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter14 -Isrc/chapter11 -o $@ $<

$(BUILDDIR)/chapter15: src/chapter15/main.cpp src/chapter15/*.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Isrc/chapter15 -o $@ $<

# --- Individual example builds (legacy) ---
build/graph_example: examples/graph_example.cpp include/ral/graph.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -o $@ $<

build/data_structures_example: examples/data_structures_example.cpp include/ral/data_structures.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -o $@ $<

build/algebraic_example: examples/algebraic_example.cpp include/ral/algebraic.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -o $@ $<

build/geometry_example: examples/geometry_example.cpp include/ral/geometry.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -o $@ $<

build/probability_example: examples/probability_example.cpp include/ral/probability.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -o $@ $<

build/modern_algorithms_example: examples/modern_algorithms_example.cpp include/ral/sketching.h include/ral/random_projection.h include/ral/randomized_linear_algebra.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -o $@ $<

build/benchmarks: examples/benchmarks.cpp include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -o $@ $<

# --- Auto-generated example builds (one per algorithm) ---
$(BUILDDIR)/example_%: examples/example_%.cpp include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -o $@ $<

# --- All examples ---
examples: $(EXAMPLE_BINS) build/graph_example build/data_structures_example build/algebraic_example build/geometry_example build/probability_example build/modern_algorithms_example build/benchmarks

# --- Tests ---
build/test_core: tests/test_core_algorithms.cpp tests/test_framework.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -Itests -o $@ $<

build/test_new: tests/test_new_algorithms.cpp tests/test_framework.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -Itests -o $@ $<

build/test_industrial: tests/test_industrial_apps.cpp tests/test_framework.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -Itests -o $@ $<

build/test_stress_number_theory: tests/test_stress_number_theory.cpp tests/test_framework.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -Itests -o $@ $<

build/test_stress_graph: tests/test_stress_graph.cpp tests/test_framework.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -Itests -o $@ $<

build/test_stress_data_structures: tests/test_stress_data_structures.cpp tests/test_framework.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -Itests -o $@ $<

build/test_stress_geometry: tests/test_stress_geometry.cpp tests/test_framework.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -Itests -o $@ $<

build/test_stress_probabilistic: tests/test_stress_probabilistic.cpp tests/test_framework.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -Itests -o $@ $<

build/test_mit_6856: tests/test_mit_6856.cpp tests/test_framework.h include/ral.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -Iinclude -Itests -o $@ $<

test: build/test_core build/test_new build/test_industrial build/test_stress_number_theory build/test_stress_graph build/test_stress_data_structures build/test_stress_geometry build/test_stress_probabilistic build/test_mit_6856
	@echo "Running core algorithm tests..."
	./build/test_core
	@echo "Running new algorithm tests..."
	./build/test_new
	@echo "Running industrial app tests..."
	./build/test_industrial
	@echo "Running stress tests (number theory)..."
	./build/test_stress_number_theory
	@echo "Running stress tests (graph)..."
	./build/test_stress_graph
	@echo "Running stress tests (data structures)..."
	./build/test_stress_data_structures
	@echo "Running stress tests (geometry)..."
	./build/test_stress_geometry
	@echo "Running stress tests (probabilistic)..."
	./build/test_stress_probabilistic
	@echo "Running MIT 6.856J tests..."
	./build/test_mit_6856

# --- Coverage ---
COVERAGE_DIR = coverage_report
GCOV_TOOL = /usr/bin/gcov

coverage:
	mkdir -p $(COVERAGE_DIR)
	$(CXX) -std=c++23 -O0 --coverage -fprofile-arcs -ftest-coverage -Iinclude -Itests -o build/test_core_gcov tests/test_core_algorithms.cpp
	$(CXX) -std=c++23 -O0 --coverage -fprofile-arcs -ftest-coverage -Iinclude -Itests -o build/test_new_gcov tests/test_new_algorithms.cpp
	$(CXX) -std=c++23 -O0 --coverage -fprofile-arcs -ftest-coverage -Iinclude -Itests -o build/test_industrial_gcov tests/test_industrial_apps.cpp
	./build/test_core_gcov
	./build/test_new_gcov
	./build/test_industrial_gcov
	lcov --capture --directory build --output-file $(COVERAGE_DIR)/coverage.info --gcov-tool $(GCOV_TOOL) --ignore-errors inconsistent,unsupported 2>/dev/null || true
	lcov --remove $(COVERAGE_DIR)/coverage.info '/usr/*' '*/tests/*' --output-file $(COVERAGE_DIR)/coverage_filtered.info --gcov-tool $(GCOV_TOOL) --ignore-errors inconsistent,unsupported 2>/dev/null || true
	genhtml $(COVERAGE_DIR)/coverage_filtered.info --output-directory $(COVERAGE_DIR)/html --ignore-errors inconsistent,unsupported,corrupt 2>/dev/null || true
	@echo ""
	@echo "=== Coverage Summary ==="
	@lcov --summary $(COVERAGE_DIR)/coverage_filtered.info --gcov-tool $(GCOV_TOOL) 2>/dev/null || true
	@echo ""
	@echo "Full report: $(COVERAGE_DIR)/html/index.html"

run-examples: examples
	./build/graph_example
	./build/data_structures_example
	./build/algebraic_example
	./build/geometry_example
	./build/probability_example
	./build/modern_algorithms_example
	./build/benchmarks

clean:
	rm -rf $(BUILDDIR) coverage_report

run: all
	@for ch in $(CHAPTERS); do echo "--- Chapter $$ch ---"; ./$(BUILDDIR)/chapter$$ch > /dev/null 2>&1 && echo "  OK" || echo "  FAILED"; done
