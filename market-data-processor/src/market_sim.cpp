#include "market_sim.hpp"
#include <chrono>

MarketSimulator::MarketSimulator()
    : price_dist_(-0.001, 0.001),   // +-0.1% price change per tick
      qty_dist_(1, 100),
      side_dist_(0, 1),
      gen_(std::random_device{}())
{
}

Tick MarketSimulator::next_tick() {
    // Random walk: multiply price by (1 + small delta)
    double change = 1.0 + price_dist_(gen_);
    current_price_ = current_price_ * change;

    Side side = static_cast<Side>(side_dist_(gen_));
    double quantity = qty_dist_(gen_);

    // Timestamp in nanoseconds since epoch
    auto now = std::chrono::system_clock::now();
    uint64_t timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()
    ).count();

    Tick tick = {};
    tick.price = current_price_;
    tick.quantity = quantity;
    tick.timestamp = timestamp_ns;
    tick.side = side;

    return tick;
}
