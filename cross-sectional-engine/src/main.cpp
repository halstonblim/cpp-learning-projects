#include "core/universe_store.hpp"
#include "ingestion/market_feed.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>

int main() {

    constexpr size_t UNIVERSE_CAPACITY = 1024;
    constexpr size_t QUEUE_SIZE = 4096;

    UniverseStore store(UNIVERSE_CAPACITY);
    MarketFeed feed(store, QUEUE_SIZE);

    feed.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    feed.stop();

    // Get raw pointers to SoA vectors
    const float* prices = store.get_prices();
    const float* bids   = store.get_bids();
    const float* asks   = store.get_asks();

    std::cout << std::fixed << std::setprecision(4);
    for (size_t i = 0; i < 5; ++i) {
        std::cout << "Asset " << i << ": "
                << "Bid=" << bids[i] << " "
                << "Ask=" << asks[i] << " "
                << "LTP=" << prices[i] 
                << std::endl;
    }

    // Quick sanity check logic
    if (prices[0] == 0.0f) {
        std::cerr << "[ERROR] Price is 0.0. Data did not reach the store!" << std::endl;
        return 1;
    }
    if (std::abs(prices[0] - 100.0f) < 0.0001f) {
        std::cerr << "[WARNING] Price is exactly 100.0. Updates might not be flowing." << std::endl;
    } else {
        std::cout << "[SUCCESS] Prices have drifted. Pipeline is active." << std::endl;
    }

    return 0;
}