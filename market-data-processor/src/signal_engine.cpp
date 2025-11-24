#include "signal_engine.hpp"
#include <iostream>
#include <chrono>
#include <algorithm> // for std::sort
#include <numeric>   // for std::accumulate

SignalEngine::SignalEngine() {
    // PRE-ALLOCATION IS CRITICAL
    // We expect ~1 million ticks. Reserving memory upfront prevents
    // the vector from reallocating (and copying) data during the hot path.
    latencies_.reserve(1'000'000); 
}

void SignalEngine::process_tick(const Tick& tick) {
    // 1. Measure "Now"
    // Note: We use system_clock to match the producer's clock. 
    // (In a real HFT system, we'd use hardware timestamps, but this is fine for now).
    auto now = std::chrono::system_clock::now();
    auto now_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()
    ).count();

    // 2. Calculate Latency (Now - CreationTime)
    long latency = now_nanos - tick.timestamp;
    
    // 3. Store it (Fast operation because we reserved memory)
    latencies_.push_back(latency);

    // 4. Update State (The Business Logic)
    total_traded_value_ += (tick.price * tick.quantity);
    total_quantity_ += tick.quantity;
    
    // Note: We do NOT calculate/print VWAP here anymore. 
    // That would slow us down.
}

void SignalEngine::write_latency_report() {
    if (latencies_.empty()) {
        std::cout << "No latencies recorded." << std::endl;
        return;
    }

    // Sort to calculate percentiles
    std::sort(latencies_.begin(), latencies_.end());

    size_t n = latencies_.size();
    long min_lat = latencies_.front();
    long max_lat = latencies_.back();
    long p50 = latencies_[n * 0.50]; // Median
    long p90 = latencies_[n * 0.90];
    long p99 = latencies_[n * 0.99]; // The "Tail"

    double sum = std::accumulate(latencies_.begin(), latencies_.end(), 0.0);
    double avg = sum / n;

    std::cout << "\n--- Latency Report (Nanoseconds) ---" << std::endl;
    std::cout << "Count: " << n << std::endl;
    std::cout << "Min:   " << min_lat << " ns" << std::endl;
    std::cout << "Avg:   " << static_cast<long>(avg) << " ns" << std::endl;
    std::cout << "P50:   " << p50 << " ns" << std::endl;
    std::cout << "P90:   " << p90 << " ns" << std::endl;
    std::cout << "P99:   " << p99 << " ns" << std::endl;
    std::cout << "Max:   " << max_lat << " ns" << std::endl;
    std::cout << "------------------------------------" << std::endl;
}