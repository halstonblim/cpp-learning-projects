#include "ingestion/market_feed.hpp"
#include <random>
#include <iostream>

// Constants for our simulation
static constexpr float INITIAL_PRICE = 100.0f;
static constexpr float VOLATILITY = 0.001f; // 0.1% move per tick

MarketFeed::MarketFeed(UniverseStore& store, size_t queue_size)
    : store_(store), queue_(queue_size) 
{
}

MarketFeed::~MarketFeed() {
    stop();
}

void MarketFeed::start() {
    if (running_) return;
    running_ = true;

    // Launch threads. 
    // note: jthreads automatically join on destruction, 
    // but explicit management is often clearer for start/stop logic.
    producer_thread_ = std::jthread([this] { producer_loop(); });
    consumer_thread_ = std::jthread([this] { consumer_loop(); });
    
    std::cout << "Market Feed started." << std::endl;
}

void MarketFeed::stop() {
    if (!running_) return;
    running_ = false;
    
    // Reset triggers auto-join for jthread, ensuring threads finish
    // before we return. This prevents data races when reading from store.
    producer_thread_ = std::jthread{};
    consumer_thread_ = std::jthread{};
}

void MarketFeed::producer_loop() {
    // 1. Setup Random Number Generation
    // We use a separate RNG per thread to avoid locking overhead.
    std::mt19937 gen(std::random_device{}());
    
    // Pick a random stock to update
    std::uniform_int_distribution<uint32_t> asset_dist(0, store_.size() - 1);
    
    // Random price movement (-0.1% to +0.1%)
    std::uniform_real_distribution<float> price_shock_dist(-VOLATILITY, VOLATILITY);
    
    // Random volume (1 to 100 shares)
    std::uniform_real_distribution<float> vol_dist(1.0f, 100.0f);

    // 2. Initialize "Exchange" State
    // The exchange knows the current price of every asset.
    std::vector<float> current_prices(store_.size(), INITIAL_PRICE);

    while (running_) {
        // A. Simulate a Trade
        uint32_t asset_id = asset_dist(gen);
        float shock = price_shock_dist(gen);
        
        // Update the "Exchange's" view of the price
        // (Geometric Brownian Motion-ish)
        current_prices[asset_id] *= (1.0f + shock);
        float price = current_prices[asset_id];

        // Create the spread (e.g., 1 cent wide)
        float bid = price - 0.01f;
        float ask = price + 0.01f;

        // B. Create the Update Packet
        MarketUpdate update{
            .asset_id = asset_id,
            .price = price,
            .volume = vol_dist(gen),
            .bid = bid,
            .ask = ask
        };

        // C. Push to Queue
        // If queue is full, we spin-yield. In a real feed, we might drop packets.
        while (!queue_.push(update) && running_) {
            std::this_thread::yield(); 
        }
        
        // Optional: Slow down slightly so we don't melt the CPU 
        // completely during testing (remove for max throughput benchmarking)
        // std::this_thread::sleep_for(std::chrono::nanoseconds(100));
    }
}

void MarketFeed::consumer_loop() {
    while (running_) {
        // A. Pop from Queue
        std::optional<MarketUpdate> update_opt = queue_.pop();

        if (update_opt) {
            // B. The Hot Path Update
            // This converts the AoS update to our SoA storage
            store_.write_lock();
            store_.update_tick(*update_opt);
            store_.write_unlock();
        } else {
            // Queue empty? Yield to let producer catch up.
            // In low-latency, we might busy-spin here (burn CPU), 
            // but yield is friendlier for dev machines.
            std::this_thread::yield();
        }
    }
}