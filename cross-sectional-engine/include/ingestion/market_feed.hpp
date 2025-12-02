#pragma once
#include <atomic>
#include <thread>
#include "core/ring_buffer.hpp"   // Your LockFreeQueue
#include "core/universe_store.hpp"

class MarketFeed {
public:
    MarketFeed(UniverseStore& store, size_t queue_size);
    ~MarketFeed();

    void start();
    void stop();

private:
    void producer_loop();
    void consumer_loop();

    UniverseStore& store_;
    LockFreeQueue<MarketUpdate> queue_;
    
    std::atomic<bool> running_{false};
    std::jthread producer_thread_;
    std::jthread consumer_thread_;
};