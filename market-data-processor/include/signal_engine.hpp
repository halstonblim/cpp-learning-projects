#pragma once
#include "types.hpp"
#include "lock_based_queue.hpp"
#include <atomic>

class SignalEngine {
private:
    // reference to shared input queue
    ThreadSafeQueue<Tick>& queue_;

    // control flag
    std::atomic<bool> running_{true};

    // quantityes to calculate
    double total_traded_value_ = 0.0; // Sum of (Price * Qty)
    double total_quantity_ = 0.0;     // Sum of Qty

public:
    SignalEngine(ThreadSafeQueue<Tick>& queue) : queue_(queue) {}

    void run();
    void stop();
};