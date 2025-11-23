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