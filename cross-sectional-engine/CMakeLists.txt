cmake_minimum_required(VERSION 3.20)
project(signal_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_library(signal_engine_lib src/ingestion/market_feed.cpp)
target_include_directories(signal_engine_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src)

include(FetchContent)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.1
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)
add_subdirectory(benchmarks)