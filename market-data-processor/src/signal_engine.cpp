#include "signal_engine.hpp"
#include <iostream>
#include <iomanip> // For std::setprecision

void SignalEngine::process_tick(const Tick& tick) {
    // STEP 1: Update State (The Math)
    total_traded_value_ += (tick.price * tick.quantity);
    total_quantity_ += tick.quantity;

    // STEP 2: Calculate VWAP
    // Guard against division by zero
    double vwap = 0.0;
    if (total_quantity_ > 0) {
        vwap = total_traded_value_ / total_quantity_;
    }

    // STEP 3: Output (The Slow Part)
    std::cout << "[SignalEngine] Time: " << tick.timestamp 
              << " | Price: " << std::fixed << std::setprecision(2) << tick.price 
              << " | VWAP: " << vwap 
              << " | Diff: " << (tick.price - vwap) << std::endl;
}
