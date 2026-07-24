CXX = g++
CXXFLAGS = -std=c++23 -O1 -Wall -Wextra -Wpedantic

BUILDDIR = build

# Chapter 2 is omitted as it only contains inline code listings in the book
CHAPTERS = 1 3 4 5 6 7 8 9 10 11 12 13 14 15

.PHONY: all clean run examples run-examples

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

examples: build/graph_example build/data_structures_example build/algebraic_example build/geometry_example build/probability_example build/modern_algorithms_example build/benchmarks

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

run-examples: examples
	./build/graph_example
	./build/data_structures_example
	./build/algebraic_example
	./build/geometry_example
	./build/probability_example
	./build/modern_algorithms_example
	./build/benchmarks

clean:
	rm -rf $(BUILDDIR)

run: all
	@for ch in $(CHAPTERS); do echo "--- Chapter $$ch ---"; ./$(BUILDDIR)/chapter$$ch > /dev/null 2>&1 && echo "  OK" || echo "  FAILED"; done
