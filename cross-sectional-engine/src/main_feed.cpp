#include "ingestion/market_feed.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>

int main() {
    constexpr size_t NUM_ASSETS = 2048;
    constexpr auto TEST_DURATION = std::chrono::seconds(5);
    constexpr auto SAMPLE_INTERVAL = std::chrono::milliseconds(10);
    
    UniverseStore store(NUM_ASSETS);
    MarketFeed feed(store, 4096);
    
    size_t max_queue_depth = 0;
    size_t sample_count = 0;
    size_t total_queue_depth = 0;
    
    auto start = std::chrono::steady_clock::now();
    feed.start();
    
    auto end_time = start + TEST_DURATION;
    while (std::chrono::steady_clock::now() < end_time) {
        std::this_thread::sleep_for(SAMPLE_INTERVAL);
        size_t depth = feed.get_queue_depth();
        max_queue_depth = std::max(max_queue_depth, depth);
        total_queue_depth += depth;
        sample_count++;
    }
    
    auto elapsed = std::chrono::steady_clock::now() - start;
    feed.stop();
    
    uint64_t updates = feed.get_updates_processed();
    double elapsed_sec = std::chrono::duration<double>(elapsed).count();
    double throughput = updates / elapsed_sec;
    double avg_queue_depth = sample_count > 0 ? 
        static_cast<double>(total_queue_depth) / sample_count : 0.0;
    
    std::cout << "\n===== MarketFeed Performance Metrics =====\n\n";
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Throughput:        " << std::setw(12) << throughput / 1e6 
              << " M updates/sec\n";
    std::cout << "Total Updates:     " << std::setw(12) << updates << "\n";
    std::cout << "Elapsed Time:      " << std::setw(12) << elapsed_sec << " sec\n\n";
    
    std::cout << "Queue Capacity:    " << std::setw(12) << feed.get_queue_capacity() << "\n";
    std::cout << "Max Queue Depth:   " << std::setw(12) << max_queue_depth 
              << " (" << std::setprecision(1) 
              << (100.0 * max_queue_depth / feed.get_queue_capacity()) << "% full)\n";
    std::cout << std::setprecision(1);
    std::cout << "Avg Queue Depth:   " << std::setw(12) << avg_queue_depth << "\n\n";
    
    return 0;
}
