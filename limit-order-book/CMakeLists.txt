# Export compile commands to make clangd happy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.15) 

# Name project
project(limit_order_book)

# Define executable and list its source files
add_executable(lob_driver
    src/main.cpp
    src/LimitOrderBook.cpp
)

# Add include directory to your target
target_include_directories(lob_driver
    PRIVATE # PRIVATE is correct here, as it's an executable
        include
)

# Set the C++ standard for your target
# This replaces the global set(CMAKE_CXX_STANDARD ...) commands
target_compile_features(lob_driver
    PRIVATE # Again, PRIVATE is for compiling this target only
        cxx_std_17 # This tells CMake to "find and use C++17 features"
)