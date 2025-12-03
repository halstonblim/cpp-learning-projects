#pragma once
#include "core/universe_store.hpp"

class SignalEngine {
public:
    SignalEngine(UniverseStore& store) : store_(store) {}

    // Previous Scalar/AVX Dot Product
    [[nodiscard]] float calculate_total_notional_scalar() const;
    [[nodiscard]] float calculate_total_notional_avx() const;
    
    // Cross-Sectional Statistics
    [[nodiscard]] float calculate_mean_avx() const;
    [[nodiscard]] float calculate_std_dev_avx(float mean) const;
    
    // Signal
    // writes z-scores to output array
    void calculate_zscores_avx(float* out_scores) const;

private:
    UniverseStore& store_;
};