#include "signal_engine.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm> // for std::sort
#include <numeric>   // for std::accumulate

SignalEngine::SignalEngine() {
    // Avoid reallocation during hot path
    latencies_.reserve(1'000'000);
}

void SignalEngine::process_tick(const Tick& tick) {
    // Measure receive time (matches producer's system_clock)
    auto now = std::chrono::system_clock::now();
    auto now_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()
    ).count();

    long latency = now_nanos - tick.timestamp;
    latencies_.push_back(latency);

    // Accumulate for VWAP calculation
    total_traded_value_ += (tick.price * tick.quantity);
    total_quantity_ += tick.quantity;
}

void SignalEngine::write_latency_report() {
    if (latencies_.empty()) {
        std::cout << "No latencies recorded." << std::endl;
        return;
    }

    // Copy for sorting (preserve original order for CSV export)
    std::vector<long> sorted_latencies = latencies_;
    std::sort(sorted_latencies.begin(), sorted_latencies.end());

    size_t n = sorted_latencies.size();
    long min_lat = sorted_latencies.front();
    long max_lat = sorted_latencies.back();
    long p50 = sorted_latencies[n * 0.50];
    long p90 = sorted_latencies[n * 0.90];
    long p99 = sorted_latencies[n * 0.99];

    double sum = std::accumulate(sorted_latencies.begin(), sorted_latencies.end(), 0.0);
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

void SignalEngine::export_latencies_csv(const std::string& filename) {
    if (latencies_.empty()) {
        std::cerr << "No latencies to export." << std::endl;
        return;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    file << "tick_index,latency_ns\n";

    // Write in arrival order (unsorted)
    for (size_t i = 0; i < latencies_.size(); ++i) {
        file << i << "," << latencies_[i] << "\n";
    }

    file.close();
    std::cout << "Exported " << latencies_.size() << " latency samples to: " << filename << std::endl;
}
