#pragma once
#include "types.hpp"

class SignalEngine {
private:
    // quantities to calculate
    double total_traded_value_ = 0.0; // Sum of (Price * Qty)
    double total_quantity_ = 0.0;     // Sum of Qty

public:
    SignalEngine() = default;

    void process_tick(const Tick& tick);
};
