add_library(market_sim market_sim.cpp)

target_include_directories(market_sim
  PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)
