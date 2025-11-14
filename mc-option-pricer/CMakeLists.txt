cmake_minimum_required(VERSION 3.20)
project(MCOptionPricer LANGUAGES CXX)

# Set the C++ standard for the whole project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# This is key! It makes clangd work.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add the 'src' directory, which contains the main library
add_subdirectory(../online-stats ${CMAKE_CURRENT_BINARY_DIR}/online-stat)

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
add_subdirectory(tests)