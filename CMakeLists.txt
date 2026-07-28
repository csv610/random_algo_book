cmake_minimum_required(VERSION 3.20)
project(RandomizedAlgorithms VERSION 1.0 LANGUAGES CXX)

# Enforce C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the header-only RAL library target
add_library(ral INTERFACE)
target_include_directories(ral INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Common compiler warning flags
if(MSVC)
    set(COMPILER_WARNINGS /W4)
else()
    set(COMPILER_WARNINGS -Wall -Wextra -Wpedantic)
endif()

# -------------------------------------------------------------
# 1. Chapter Runners (Legacy Verifications)
# -------------------------------------------------------------
set(CHAPTERS 1 3 4 5 6 7 8 9 10 11 12 13 14 15)
foreach(CH ${CHAPTERS})
    add_executable(chapter${CH} src/chapter${CH}/main.cpp)
    target_compile_options(chapter${CH} PRIVATE ${COMPILER_WARNINGS})
    
    # Chapter 14 depends on Chapter 11 random_utils.h in legacy forwarding layout
    if(CH EQUAL 14)
        target_include_directories(chapter14 PRIVATE src/chapter14 src/chapter11)
    else()
        target_include_directories(chapter${CH} PRIVATE src/chapter${CH})
    endif()
endforeach()

# -------------------------------------------------------------
# 2. Examples & Benchmarks
# -------------------------------------------------------------
set(EXAMPLES
    karger_min_cut
    karger_stein_min_cut
    las_vegas_monte_carlo
    binary_planar_partition
    randomized_find
    graph_example
    data_structures_example
    algebraic_example
    geometry_example
    probability_example
    modern_algorithms_example
    benchmarks
    example_karger_min_cut
    example_karger_stein
    example_exact_min_cut
    example_kruskal_mst
    example_kkt_mst
    example_floyd_warshall
    example_seidel_apsp
    example_apsp_repeated_squaring
    example_bool_matrix_multiply
    example_blossom_matching
    example_bipartite_matching
    example_packet_routing
    example_random_cut
    example_max_flow_bfs
    example_multigraph
    example_chernoff_bound
    example_chernoff_simulation
    example_coupon_collector
    example_azuma_inequality
    example_potential_method
    example_k_smaller_recurrence
    example_linear_expectation
    example_randomized_quicksort
    example_monte_carlo_min_cut
    example_monte_carlo_pi
    example_oblivious_adversary
    example_adaptive_adversary
)

foreach(EX ${EXAMPLES})
    add_executable(${EX} examples/${EX}.cpp)
    target_compile_options(${EX} PRIVATE ${COMPILER_WARNINGS})
    target_link_libraries(${EX} PRIVATE ral)
endforeach()