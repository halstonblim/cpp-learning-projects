#include "signal_engine.hpp"
#include <iostream>
#include <iomanip> // For std::setprecision

void SignalEngine::run() {
    Tick tick = {};
    
    std::cout << "Signal Engine started. Waiting for market data..." << std::endl;

    while (running_) {
        // STEP 1: Blocking Pop
        // This line halts execution if the queue is empty.
        // The thread goes to "Sleep" state (0% CPU usage) until notified.
        queue_.wait_and_pop(tick);

        // STEP 2: Update State (The Math)
        total_traded_value_ += (tick.price * tick.quantity);
        total_quantity_ += tick.quantity;

        // STEP 3: Calculate VWAP
        // Guard against division by zero
        double vwap = 0.0;
        if (total_quantity_ > 0) {
            vwap = total_traded_value_ / total_quantity_;
        }

        // STEP 4: Output (The Slow Part)
        std::cout << "[SignalEngine] Time: " << tick.timestamp 
                  << " | Price: " << std::fixed << std::setprecision(2) << tick.price 
                  << " | VWAP: " << vwap 
                  << " | Diff: " << (tick.price - vwap) << std::endl;
    }
}

void SignalEngine::stop() {
    running_ = false;
    // Note: If the thread is stuck in wait_and_pop, setting this to false
    // won't wake it up immediately. We usually push a dummy "poison pill" tick
    // to force it to wake up and check the flag. For Day 1, Ctrl+C is fine.
}