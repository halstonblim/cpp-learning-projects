#pragma once
#include "types.hpp"
#include <random>

class MarketSimulator {
private:
    // 1. Simulation State
    double current_price_ = 100.00;

    // 2. Random Number Generation (Initialized in constructor)
    std::mt19937 gen_;
    std::uniform_real_distribution<> price_dist_; // e.g., -0.01 to +0.01
    std::uniform_int_distribution<> qty_dist_;    // e.g., 1 to 100
    std::uniform_int_distribution<> side_dist_;   // 0 to 1

public:
    // Constructor: Seeds the RNG
    MarketSimulator();

    // Generate next tick (pure function effectively, updates internal state)
    Tick next_tick();
};
