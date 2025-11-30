cmake_minimum_required(VERSION 3.20)
project(simd_timeseries CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.1
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

include_directories(include)

# NOTE: This project specifically targets AVX2 for learning purposes.
# It will not compile/run on systems without AVX2 support.
add_library(timeseries_lib src/time_series.cpp)
target_compile_options(timeseries_lib PUBLIC -mavx2)

enable_testing()

add_subdirectory(tests)
add_subdirectory(benchmarks)