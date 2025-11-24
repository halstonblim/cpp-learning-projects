#include "market_sim.hpp"
#include <chrono>

MarketSimulator::MarketSimulator() 
    : price_dist_(-0.001, 0.001),
      qty_dist_(1, 100),
      side_dist_(0, 1),
      gen_(std::random_device{}())
{
}

Tick MarketSimulator::next_tick() {
    double change = 1.0 + price_dist_(gen_);
    current_price_ = current_price_ * change;
    Side side = static_cast<Side>(side_dist_(gen_));
    double quantity = qty_dist_(gen_);

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    uint64_t timestampnanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();                
    
    Tick tick = {};
    tick.price = current_price_;
    tick.quantity = quantity;
    tick.timestamp = timestampnanos;
    tick.side = side;

    return tick;
}
