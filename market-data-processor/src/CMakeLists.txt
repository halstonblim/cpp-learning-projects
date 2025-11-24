add_library(market_sim market_sim.cpp signal_engine.cpp)

target_include_directories(market_sim
  PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)

add_executable(market_simulator main.cpp)
target_link_libraries(market_simulator PRIVATE market_sim)
