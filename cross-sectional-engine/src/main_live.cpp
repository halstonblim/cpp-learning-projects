#include "ingestion/market_feed.hpp"
#include "compute/signal_engine.hpp"
#include "strategy/zscore_strategy.hpp"
#include "core/aligned_allocator.hpp"

#include <chrono>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>
#include <cmath>

struct LiveMetrics {
    uint64_t total_cycles{0};
    uint64_t buy_signals{0};
    uint64_t sell_signals{0};
    uint64_t hold_signals{0};
    std::vector<double> cycle_latencies_us;
    
    void record_cycle(double latency_us, size_t buys, size_t sells, size_t holds) {
        cycle_latencies_us.push_back(latency_us);
        buy_signals += buys;
        sell_signals += sells;
        hold_signals += holds;
        ++total_cycles;
    }
    
    void print_summary() const {
        if (cycle_latencies_us.empty()) {
            std::cout << "No cycles recorded.\n";
            return;
        }
        
        std::vector<double> sorted_latencies = cycle_latencies_us;
        std::sort(sorted_latencies.begin(), sorted_latencies.end());
        
        double sum = std::accumulate(sorted_latencies.begin(), sorted_latencies.end(), 0.0);
        double mean = sum / sorted_latencies.size();
        
        double sq_sum = 0.0;
        for (double lat : sorted_latencies) {
            sq_sum += (lat - mean) * (lat - mean);
        }
        double std_dev = std::sqrt(sq_sum / sorted_latencies.size());
        
        size_t p50_idx = sorted_latencies.size() / 2;
        size_t p99_idx = static_cast<size_t>(sorted_latencies.size() * 0.99);
        size_t p999_idx = static_cast<size_t>(sorted_latencies.size() * 0.999);
        
        std::cout << "\n===== Live Trading Loop Metrics =====\n\n";
        std::cout << std::fixed << std::setprecision(2);
        
        std::cout << "Signal Computation Latency:\n";
        std::cout << "  Mean:     " << std::setw(10) << mean << " μs\n";
        std::cout << "  Std Dev:  " << std::setw(10) << std_dev << " μs\n";
        std::cout << "  Min:      " << std::setw(10) << sorted_latencies.front() << " μs\n";
        std::cout << "  p50:      " << std::setw(10) << sorted_latencies[p50_idx] << " μs\n";
        std::cout << "  p99:      " << std::setw(10) << sorted_latencies[p99_idx] << " μs\n";
        std::cout << "  p99.9:    " << std::setw(10) << sorted_latencies[p999_idx] << " μs\n";
        std::cout << "  Max:      " << std::setw(10) << sorted_latencies.back() << " μs\n\n";
        
        std::cout << "Signal Distribution:\n";
        std::cout << "  Total Cycles: " << total_cycles << "\n";
        std::cout << "  Buy Signals:  " << buy_signals << " (" 
                  << (100.0 * buy_signals / (buy_signals + sell_signals + hold_signals)) << "%)\n";
        std::cout << "  Sell Signals: " << sell_signals << " ("
                  << (100.0 * sell_signals / (buy_signals + sell_signals + hold_signals)) << "%)\n";
        std::cout << "  Hold Signals: " << hold_signals << " ("
                  << (100.0 * hold_signals / (buy_signals + sell_signals + hold_signals)) << "%)\n\n";
    }
};

int main() {
    constexpr size_t NUM_ASSETS = 2048;
    constexpr size_t QUEUE_SIZE = 4096;
    constexpr auto RUN_DURATION = std::chrono::seconds(10);
    constexpr auto SIGNAL_INTERVAL = std::chrono::microseconds(100); // 10kHz signal rate
    
    // Initialize components
    UniverseStore store(NUM_ASSETS);
    SignalEngine engine(store);
    MarketFeed feed(store, QUEUE_SIZE);
    
    // Aligned buffers for signal computation
    std::vector<float, AlignedAllocator<float, 32>> zscores(NUM_ASSETS);
    std::vector<float, AlignedAllocator<float, 32>> signals(NUM_ASSETS);
    
    LiveMetrics metrics;
    metrics.cycle_latencies_us.reserve(RUN_DURATION.count() * 10000); // Pre-allocate
    
    std::cout << "Starting live trading simulation...\n";
    std::cout << "  Assets:          " << NUM_ASSETS << "\n";
    std::cout << "  Signal Interval: " << SIGNAL_INTERVAL.count() << " μs\n";
    std::cout << "  Duration:        " << RUN_DURATION.count() << " s\n\n";
    
    // Start market data feed (producer/consumer threads)
    feed.start();
    
    // Let the feed warm up and populate the store
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + RUN_DURATION;
    
    // Main trading loop
    while (std::chrono::steady_clock::now() < end_time) {
        auto cycle_start = std::chrono::high_resolution_clock::now();
        
        // Reads data from store via seqlock
        engine.calculate_zscores_avx(zscores.data());
        
        // 2. Generate trading signals
        ZScoreStrategy::calculate({zscores.data()}, signals.data(), NUM_ASSETS);
        
        auto cycle_end = std::chrono::high_resolution_clock::now();
        
        // 3. Count signals
        size_t buys = 0, sells = 0, holds = 0;
        for (size_t i = 0; i < NUM_ASSETS; ++i) {
            if (signals[i] > 0.5f) ++buys;
            else if (signals[i] < -0.5f) ++sells;
            else ++holds;
        }
        
        // 4. Record metrics
        double latency_us = std::chrono::duration<double, std::micro>(cycle_end - cycle_start).count();
        metrics.record_cycle(latency_us, buys, sells, holds);
        
        // 5. Wait for next cycle (simulating fixed-rate signal generation)
        std::this_thread::sleep_for(SIGNAL_INTERVAL);
    }
    
    // Shutdown
    feed.stop();
    
    // Report
    auto actual_duration = std::chrono::steady_clock::now() - start_time;
    double actual_sec = std::chrono::duration<double>(actual_duration).count();
    
    std::cout << "Feed processed " << feed.get_updates_processed() 
              << " updates in " << std::fixed << std::setprecision(2) << actual_sec << "s\n";
    std::cout << "Throughput: " << (feed.get_updates_processed() / actual_sec / 1e6) 
              << " M updates/sec\n";
    
    metrics.print_summary();
    
    return 0;
}