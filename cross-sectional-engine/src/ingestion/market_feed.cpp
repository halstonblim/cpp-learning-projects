#include "ingestion/market_feed.hpp"
#include <random>
#include <iostream>

static constexpr float INITIAL_PRICE = 100.0f;
static constexpr float VOLATILITY = 0.001f;

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
    producer_thread_ = std::jthread([this] { producer_loop(); });
    consumer_thread_ = std::jthread([this] { consumer_loop(); });
    std::cout << "Market Feed started." << std::endl;
}

void MarketFeed::stop() {
    if (!running_) return;
    running_ = false;
    producer_thread_ = std::jthread{};
    consumer_thread_ = std::jthread{};
}

void MarketFeed::producer_loop() {
    std::mt19937 gen(std::random_device{}());
    
    if (store_.capacity() == 0) {
        return;
    }
    std::uniform_int_distribution<uint32_t> asset_dist(0, static_cast<uint32_t>(store_.capacity() - 1));
    std::uniform_real_distribution<float> price_shock_dist(-VOLATILITY, VOLATILITY);
    std::uniform_real_distribution<float> vol_dist(1.0f, 100.0f);

    std::vector<float> current_prices(store_.capacity(), INITIAL_PRICE);

    while (running_) {
        uint32_t asset_id = asset_dist(gen);
        float shock = price_shock_dist(gen);
        current_prices[asset_id] *= (1.0f + shock);
        float price = current_prices[asset_id];
        float bid = price - 0.01f;
        float ask = price + 0.01f;

        MarketUpdate update{
            .asset_id = asset_id,
            .price = price,
            .volume = vol_dist(gen),
            .bid = bid,
            .ask = ask
        };

        while (!queue_.push(update) && running_) {
            std::this_thread::yield(); 
        }
    }
}

void MarketFeed::consumer_loop() {
    while (running_) {
        std::optional<MarketUpdate> update_opt = queue_.pop();

        if (update_opt) {
            store_.write_lock();
            store_.update_tick(*update_opt);
            store_.write_unlock();
            updates_processed_.fetch_add(1, std::memory_order_relaxed);
        } else {
            std::this_thread::yield();
        }
    }
}