cmake_minimum_required(VERSION 3.10)
project(WelfordAlgorithm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(OnlineStats INTERFACE)

target_include_directories(OnlineStats
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)

add_subdirectory(tests)