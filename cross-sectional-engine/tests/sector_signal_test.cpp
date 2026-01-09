#include "signals/sector_signal.hpp"
#include "core/aligned_allocator.hpp"
#include "signals/zscore_signal.hpp"
#include <gtest/gtest.h>
#include <cmath>

TEST(SectorNeutralSignalTest, ZScoresAreRelativeToSectorMean) {
    std::vector<uint32_t> assignments = {0, 0, 0, 0, 1, 1, 1, 1};
    SectorIndex index(8, 2, assignments);
    
    std::vector<float, AlignedAllocator<float, 32>> sorted_prices = {
        100.0f, 102.0f, 98.0f, 100.0f,
        50.0f, 52.0f, 48.0f, 50.0f
    };
    
    std::vector<float, AlignedAllocator<float, 32>> output(8);
    SectorNeutralSignal::calculate(sorted_prices.data(), 8, index, output.data());
    
    float tech_sum = output[0] + output[1] + output[2] + output[3];
    float bank_sum = output[4] + output[5] + output[6] + output[7];
    
    EXPECT_NEAR(tech_sum, 0.0f, 1e-5);
    EXPECT_NEAR(bank_sum, 0.0f, 1e-5);
    
    float std = std::sqrt(2.0f);
    size_t i = 0;
    for (; i < 4; ++i) EXPECT_NEAR(output[i], (sorted_prices[i] - 100.0f) / std, 1e-5);
    for (; i < 8; ++i) EXPECT_NEAR(output[i], (sorted_prices[i] - 50.0f) / std, 1e-5);        
    
    std::vector<float, AlignedAllocator<float, 32>> output_global(8);
    ZScoreSignal::calculate({sorted_prices.data(), nullptr, 8}, output_global.data());
    i = 0;
    for (; i < 4; ++i) EXPECT_GT(output_global[i], 0);
    for (; i < 8; ++i) EXPECT_LT(output_global[i], 0);
}