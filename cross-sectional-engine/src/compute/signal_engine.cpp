#include "compute/signal_engine.hpp"

float SignalEngine::calculate_total_notional_scalar() const {    
    const float* prices = store_.get_prices();
    const float* volumes = store_.get_volumes();

    float total_notional{};
    for (size_t i = 0; i < store_.size(); ++i) {
        total_notional += prices[i] * volumes[i];
    }
    return total_notional;
}