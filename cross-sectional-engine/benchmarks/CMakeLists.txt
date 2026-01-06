add_executable(aos_vs_soa_benchmark aos_vs_soa_benchmark.cpp)

target_link_libraries(aos_vs_soa_benchmark 
    PRIVATE 
    benchmark::benchmark
)

add_executable(signal_engine_benchmark signal_engine_benchmark.cpp)

target_link_libraries(signal_engine_benchmark 
    PRIVATE 
    signal_engine_lib
    benchmark::benchmark
)

add_executable(pipeline_benchmark pipeline_benchmark.cpp)

target_link_libraries(pipeline_benchmark 
    PRIVATE 
    signal_engine_lib
    benchmark::benchmark
)