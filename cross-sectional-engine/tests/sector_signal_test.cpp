#include "signals/sector_signal.hpp"
#include "core/aligned_allocator.hpp"
#include "signals/zscore_signal.hpp"
#include <gtest/gtest.h>
#include <cmath>

TEST(SectorNeutralSignalTest, ZScoresAreRelativeToSectorMean) {
    // Setup: 8 assets, 2 sectors (4 each), ALREADY SORTED by sector
    // Sector 0 (Tech): prices [100, 102, 98, 100] - mean = 100
    // Sector 1 (Bank): prices [50, 52, 48, 50]   - mean = 50
    
    std::vector<uint32_t> assignments = {0, 0, 0, 0, 1, 1, 1, 1};  // Already sorted!
    SectorIndex index(8, 2, assignments);
    
    // Prices in sorted order (Tech first, then Bank)
    std::vector<float, AlignedAllocator<float, 32>> sorted_prices = {
        100.0f, 102.0f, 98.0f, 100.0f,  // Tech sector
        50.0f, 52.0f, 48.0f, 50.0f       // Bank sector
    };
    
    std::vector<float, AlignedAllocator<float, 32>> output(8);
    
    SectorNeutralSignal::calculate(sorted_prices.data(), 8, index, output.data());
    
    // Key insight: Even though Tech prices (~100) are much higher than Bank (~50),
    // both sectors should have z-scores centered around 0 because we normalize
    // within each sector.
    
    // Test 1: Verify sector z-scores sum to ~0 (mean-centered)
    float tech_sum = output[0] + output[1] + output[2] + output[3];
    float bank_sum = output[4] + output[5] + output[6] + output[7];
    
    EXPECT_NEAR(tech_sum, 0.0f, 1e-5);
    EXPECT_NEAR(bank_sum, 0.0f, 1e-5);
    
    // Test 2: Verify specific z-scores
    // Tech: mean=100, values are [100, 102, 98, 100]
    float std = std::sqrt(2.0f); // same for tech and banks
    size_t i = 0;
    for (; i < 4; ++i) EXPECT_NEAR(output[i], (sorted_prices[i] - 100.0f) / std , 1e-5);
    for (; i < 8; ++i) EXPECT_NEAR(output[i], (sorted_prices[i] - 50.0f) / std , 1e-5);        
    
    // Test 3: Compare to global z-score (should be DIFFERENT)
    // If we used global mean (~75), Tech stocks would all have positive z-scores
    // and Bank stocks would all have negative z-scores. Sector-neutral fixes this.
    std::vector<float, AlignedAllocator<float, 32>> output_global(8);
    ZScoreSignal::calculate({sorted_prices.data(), nullptr, 8}, output_global.data());
    i = 0;
    for (; i < 4; ++i) EXPECT_GT(output_global[i], 0);
    for (; i < 8; ++i) EXPECT_LT(output_global[i], 0);
}