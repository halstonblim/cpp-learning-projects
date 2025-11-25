#pragma once
#include "types.hpp"
#include <vector>
#include <string>

class SignalEngine {
private:
    double total_traded_value_ = 0.0;
    double total_quantity_ = 0.0;
    std::vector<long> latencies_;   // Per-tick latency in nanoseconds

public:
    SignalEngine();  // Pre-allocates latency storage

    void process_tick(const Tick& tick);

    // Output latency percentiles to console
    void write_latency_report();

    // Export tick_index,latency_ns pairs to CSV
    void export_latencies_csv(const std::string& filename);
};
