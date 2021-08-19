cmake_minimum_required (VERSION 2.6)
project (player)

set(CMAKE_CXX_FLAGS "-pg -Wall -std=c++14 -O2 -pthread")

add_subdirectory(benchmarks)
add_subdirectory(player)
add_subdirectory(tests)
