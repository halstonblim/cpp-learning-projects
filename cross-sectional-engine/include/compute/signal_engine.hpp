#pragma once
#include "core/universe_store.hpp"
#include "core/aligned_allocator.hpp"

// Result struct for atomic statistics computation
// Best practice: return both values computed on the same snapshot
struct PriceStats {
    float mean{0.0f};
    float std_dev{0.0f};
};

class SignalEngine {
public:
    SignalEngine(UniverseStore& store) : store_(store) {}

    // Previous Scalar/AVX Dot Product (seqlock protected)
    [[nodiscard]] float calculate_total_notional_scalar() const;
    [[nodiscard]] float calculate_total_notional_avx() const;
    
    // Cross-Sectional Statistics (seqlock protected)
    // PREFERRED: Returns mean and std_dev computed atomically on same snapshot
    [[nodiscard]] PriceStats calculate_stats_avx() const;
    
    // Individual accessors (for convenience when only one stat needed)
    [[nodiscard]] float calculate_mean_avx() const;
    
    // Signal (seqlock protected)
    // writes z-scores to output array
    void calculate_zscores_avx(float* out_scores) const;

protected:
    // Copy current data into thread-local buffer
    void snapshot() const;

    // Mutable because snapshot() modifies these in const methods
    mutable std::vector<float, AlignedAllocator<float, 32>> snapshot_prices_;
    mutable std::vector<float, AlignedAllocator<float, 32>> snapshot_volumes_;
    mutable size_t snapshot_size_{0};

private:
    // Internal implementations - NO seqlock, operates on snapshot buffer
    [[nodiscard]] float mean_internal() const;
    [[nodiscard]] float std_dev_internal(float mean) const;
    void zscores_internal(float* out_scores) const;

    UniverseStore& store_;
};