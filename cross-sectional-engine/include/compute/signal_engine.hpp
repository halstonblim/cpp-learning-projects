#pragma once
#include "core/universe_store.hpp"
#include "core/aligned_allocator.hpp"

struct PriceStats {
    float mean{0.0f};
    float std_dev{0.0f};
};

class SignalEngine {
public:
    SignalEngine(UniverseStore& store) : store_(store) {}

    [[nodiscard]] float calculate_total_notional_scalar() const;
    [[nodiscard]] float calculate_total_notional_avx() const;
    [[nodiscard]] PriceStats calculate_stats_avx() const;
    [[nodiscard]] float calculate_mean_avx() const;
    void calculate_zscores_avx(float* out_scores) const;

protected:
    void snapshot() const;

    mutable std::vector<float, AlignedAllocator<float, 32>> snapshot_prices_;
    mutable std::vector<float, AlignedAllocator<float, 32>> snapshot_volumes_;
    mutable size_t snapshot_size_{0};

private:
    [[nodiscard]] float mean_internal() const;
    [[nodiscard]] float std_dev_internal(float mean) const;
    void zscores_internal(float* out_scores) const;

    UniverseStore& store_;
};