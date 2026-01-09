cmake_minimum_required(VERSION 3.20)
project(signal_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_library(signal_engine_lib src/ingestion/market_feed.cpp src/compute/signal_engine.cpp src/backtest/backtester.cpp src/core/sector_index.cpp)
target_include_directories(signal_engine_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_options(signal_engine_lib PUBLIC -mavx2 -mfma)

add_subdirectory(src)

include(FetchContent)

enable_testing()
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip)
FetchContent_MakeAvailable(googletest)
add_subdirectory(tests)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.1
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)
add_subdirectory(benchmarks)