cmake_minimum_required(VERSION 3.16)
project(RandomizedAlgorithms VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Chapter 1
add_executable(chapter1 
    src/chapter1/main.cpp
)

target_include_directories(chapter1 PRIVATE src/chapter1)

# Enable warnings
target_compile_options(chapter1 PRIVATE -Wall -Wextra -Wpedantic)