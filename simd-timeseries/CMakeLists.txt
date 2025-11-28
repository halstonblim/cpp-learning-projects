cmake_minimum_required(VERSION 3.20)
project(simd_timeseries CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

include_directories(include)

add_library(timeseries_lib src/time_series.cpp)

# We will add executables later
add_subdirectory(tests)