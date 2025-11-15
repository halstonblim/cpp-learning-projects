add_executable(test_mc_option_pricer test_mc_option_pricer.cpp)

target_link_libraries(test_mc_option_pricer
    PRIVATE
        mc_option_pricer
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_mc_option_pricer)

