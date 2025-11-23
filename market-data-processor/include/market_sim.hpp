#pragma once
#include "types.hpp"
#include "lock_based_queue.hpp"
#include <random>
#include <atomic>

class MarketSimulator {
private:
    // 1. Reference to the shared queue
    ThreadSafeQueue<Tick>& queue_;

    // 2. Simulation State
    double current_price_ = 100.00;
    std::atomic<bool> running_{true}; // Thread-safe flag to stop the loop

    // 3. Random Number Generation (Initialized in constructor)
    std::mt19937 gen_;
    std::uniform_real_distribution<> price_dist_; // e.g., -0.01 to +0.01
    std::uniform_int_distribution<> qty_dist_;    // e.g., 1 to 100
    std::uniform_int_distribution<> side_dist_;   // 0 to 1

public:
    // Constructor: Takes the queue reference and seeds the RNG
    MarketSimulator(ThreadSafeQueue<Tick>& queue);

    // The Main Loop
    void run();

    // Helper to stop the thread gracefully
    void stop();
};