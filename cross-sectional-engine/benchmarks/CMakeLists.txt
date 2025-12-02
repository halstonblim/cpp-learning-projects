add_executable(aos_vs_soa_benchmark aos_vs_soa_benchmark.cpp)

target_link_libraries(aos_vs_soa_benchmark 
    PRIVATE 
    benchmark::benchmark
)