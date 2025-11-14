add_library(mc_option_pricer mc_option_pricer.cpp)

target_include_directories(mc_option_pricer
  PUBLIC
    ../include
)

target_link_libraries(mc_option_pricer
  PUBLIC
    OnlineStats
)
