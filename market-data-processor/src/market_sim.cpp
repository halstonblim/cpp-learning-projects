#include "lock_based_queue.hpp"
#include <chrono>
#include <thread>
#include "market_sim.hpp"

MarketSimulator::MarketSimulator(ThreadSafeQueue<Tick>& queue) 
    : queue_(queue), 
      price_dist_(-0.001, 0.001),  // Cleaner syntax: passes args directly to constructor
      qty_dist_(1, 100),
      side_dist_(0, 1),
      gen_(std::random_device{}())
{
}

void MarketSimulator::run() {
    running_ = true;
    current_price_ = 100.0;

    double change;
    double quantity;
    Side side;
    uint64_t timestampnanos;
    Tick tick = {};


    while(running_) {
        change = 1.0 + price_dist_(gen_);
        current_price_ = current_price_ * change;
        side = static_cast<Side>(side_dist_(gen_));
        quantity = qty_dist_(gen_);

        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        timestampnanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();                
        tick.price = current_price_;
        tick.quantity = quantity;
        tick.timestamp = timestampnanos;
        tick.side = side;
        queue_.push(tick);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
}

void MarketSimulator::stop() {
    running_ = false;
}
