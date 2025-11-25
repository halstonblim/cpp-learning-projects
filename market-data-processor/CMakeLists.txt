cmake_minimum_required(VERSION 3.20)
project(MarketSimulator LANGUAGES CXX)

# Set the C++ standard for the whole project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# This is key! It makes clangd work.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

# --- GoogleTest Setup ---
enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

# Add the 'tests' directory
# add_subdirectory(tests)

# --- Google Benchmark Setup ---
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(googlebenchmark)

# --- Benchmark Executable ---
# We create a dedicated executable just for measuring performance
add_executable(singlethread_baseline benchmarks/singlethread_baseline.cpp)

# Link against benchmark library and threads
target_link_libraries(singlethread_baseline PRIVATE benchmark::benchmark pthread)

# Include our headers (so we can find "lock_free_queue.hpp")
target_include_directories(singlethread_baseline PRIVATE include)

# --- Full System Simulation Benchmark ---
add_executable(queue_benchmark benchmarks/queue_benchmark.cpp)

target_link_libraries(queue_benchmark 
    PRIVATE 
    market_sim  # Links to your library (SignalEngine, MarketSim)
    pthread
)

# Ensure it finds your headers
target_include_directories(queue_benchmark PRIVATE include)