#pragma once
#include "types.hpp"
#include <random>

// Generates synthetic market ticks with random walk pricing.
class MarketSimulator {
private:
    double current_price_ = 100.00;

    std::mt19937 gen_;
    std::uniform_real_distribution<> price_dist_;   // Price change multiplier
    std::uniform_int_distribution<> qty_dist_;      // Trade quantity
    std::uniform_int_distribution<> side_dist_;     // BUY (0) or SELL (1)

public:
    MarketSimulator();

    // Generates next tick, advancing internal state
    Tick next_tick();
};
