#pragma once
#include "types.hpp"
#include <vector>

class SignalEngine {
private:
    // Quantities to calculate
    double total_traded_value_ = 0.0; 
    double total_quantity_ = 0.0;     

    // Latency Measurement
    // We store the latency (in nanoseconds) for every tick here.
    std::vector<long> latencies_;

public:
    SignalEngine(); // Constructor is now required to reserve memory

    void process_tick(const Tick& tick);
    
    // New method to print stats after the run is complete
    void write_latency_report(); 
};