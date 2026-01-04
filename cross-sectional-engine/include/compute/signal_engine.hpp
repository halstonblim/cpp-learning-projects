#pragma once
#include "core/universe_store.hpp"

class SignalEngine {
public:
    SignalEngine(UniverseStore& store) : store_(store) {}

    // Previous Scalar/AVX Dot Product (seqlock protected)
    [[nodiscard]] float calculate_total_notional_scalar() const;
    [[nodiscard]] float calculate_total_notional_avx() const;
    
    // Cross-Sectional Statistics (seqlock protected)
    [[nodiscard]] float calculate_mean_avx() const;
    [[nodiscard]] float calculate_std_dev_avx(float mean) const;
    
    // Signal (seqlock protected)
    // writes z-scores to output array
    void calculate_zscores_avx(float* out_scores) const;

private:
    // Internal implementations - NO seqlock, caller must hold lock
    [[nodiscard]] float mean_internal() const;
    [[nodiscard]] float std_dev_internal(float mean) const;
    void zscores_internal(float* out_scores) const;

    UniverseStore& store_;
};