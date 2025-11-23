add_executable(market_sim_test market_sim_test.cpp)

# Link GoogleTest and your main library
target_link_libraries(market_sim_test
    PRIVATE
    market_sim
    GTest::gtest_main
)

# Register the test so 'ctest' can find it
include(GoogleTest)
gtest_discover_tests(market_sim_test)