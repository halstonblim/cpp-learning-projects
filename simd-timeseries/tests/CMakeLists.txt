add_executable(stats_test stats_test.cpp)

target_link_libraries(stats_test PRIVATE timeseries_lib GTest::gtest_main)