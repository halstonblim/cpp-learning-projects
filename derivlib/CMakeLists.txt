# derivlib/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(derivlib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Define the Library ---
add_library(derivlib
    src/mc/BlackScholesModel.cpp
    src/MonteCarloEngine.cpp
)

# Public headers are in 'include/'
target_include_directories(derivlib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- GoogleTest Setup ---
enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

# --- Tests ---
add_subdirectory(tests)