#pragma once
#include "core/sector_index.hpp"
#include "math/avx_math.hpp"

// Computes z-scores relative to sector mean (prices must be sector-sorted)
// Uses unaligned AVX since sector boundaries may not be 32-byte aligned
struct SectorNeutralSignal {
    static void calculate(
        const float* sorted_prices,
        [[maybe_unused]] size_t num_assets,
        const SectorIndex& sector_index,
        float* output
    ) {
        for (size_t i = 0; i < sector_index.num_sectors(); ++i) {
            size_t start = sector_index.sector_start(i);
            size_t size = sector_index.sector_end(i) - start;

            float mean = Math::avx_mean_unaligned(&sorted_prices[start], size);
            float std = Math::avx_std_dev_unaligned(&sorted_prices[start], size, mean);
            Math::avx_zscore_unaligned(&sorted_prices[start], size, mean, std, &output[start]);
        }
    }
};