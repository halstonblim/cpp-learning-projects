include(GoogleTest)

add_executable(stats_test stats_test.cpp)
add_executable(alignment_test alignment_test.cpp)
add_executable(simd_test simd_test.cpp)

target_link_libraries(stats_test PRIVATE timeseries_lib GTest::gtest_main)
target_link_libraries(alignment_test PRIVATE timeseries_lib GTest::gtest_main)
target_link_libraries(simd_test PRIVATE timeseries_lib GTest::gtest_main)

gtest_discover_tests(stats_test)
gtest_discover_tests(alignment_test)
gtest_discover_tests(simd_test)