add_executable(derivlib_test MonteCarlo_test.cpp)

target_link_libraries(derivlib_test
    PRIVATE
        derivlib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(derivlib_test)
