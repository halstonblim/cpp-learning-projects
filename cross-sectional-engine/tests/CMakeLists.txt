add_executable(signal_engine_test signal_engine_test.cpp)

target_link_libraries(signal_engine_test
    PRIVATE
        signal_engine_lib
        GTest::gtest_main
)

add_executable(snapshot_test snapshot_test.cpp)

target_link_libraries(snapshot_test
    PRIVATE
        signal_engine_lib
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(signal_engine_test)
gtest_discover_tests(snapshot_test)
