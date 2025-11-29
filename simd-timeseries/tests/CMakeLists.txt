add_executable(stats_test stats_test.cpp)
add_executable(alignment_test alignment_test.cpp)

target_link_libraries(stats_test PRIVATE timeseries_lib GTest::gtest_main)
target_link_libraries(alignment_test PRIVATE timeseries_lib GTest::gtest_main)