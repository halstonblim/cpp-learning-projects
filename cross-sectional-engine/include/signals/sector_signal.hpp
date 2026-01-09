#pragma once
#include "core/sector_index.hpp"
#include "math/avx_math.hpp"

struct SectorNeutralSignal {
    // Note: This signal needs extra context (the SectorIndex), so we use a 
    // different interface than the simple SignalStrategy concept.
    // The prices array is assumed to be in SORTED order (by sector).
    
    static void calculate(
        const float* sorted_prices,  // Prices in sector-sorted order
        size_t num_assets,
        const SectorIndex& sector_index,
        float* output                // Output z-scores (same order as input)
    ) {
        for (size_t i = 0; i < sector_index.num_sectors(); ++i) {
            size_t sector_start = sector_index.sector_start(i);
            size_t sector_size = sector_index.sector_end(i) - sector_start;

            float sector_mean = Math::avx_mean(&sorted_prices[sector_start], sector_size);
            float sector_std = Math::avx_std_dev(&sorted_prices[sector_start], sector_size, sector_mean);

            // Compute z-scores for this sector using shared AVX implementation
            Math::avx_zscore(&sorted_prices[sector_start], sector_size, sector_mean, sector_std, &output[sector_start]);
        }
    }
};