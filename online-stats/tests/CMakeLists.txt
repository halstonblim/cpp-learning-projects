add_executable(run_tests main_test.cpp)

target_link_libraries(run_tests
    PRIVATE
        OnlineStats
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(run_tests)