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
    
    uint64_t get_updates_processed() const { 
        return updates_processed_.load(); 
    }
    
    size_t get_queue_depth() const {
        return queue_.size();
    }
    
    size_t get_queue_capacity() const {
        return queue_.capacity();
    }

private:
    void producer_loop();
    void consumer_loop();

    UniverseStore& store_;
    LockFreeQueue<MarketUpdate> queue_;
    
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> updates_processed_{0};
    std::jthread producer_thread_;
    std::jthread consumer_thread_;
};