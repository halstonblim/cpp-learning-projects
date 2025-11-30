add_executable(stats_benchmark stats_benchmark.cpp)

target_link_libraries(stats_benchmark 
    PRIVATE 
    timeseries_lib 
    benchmark::benchmark
)