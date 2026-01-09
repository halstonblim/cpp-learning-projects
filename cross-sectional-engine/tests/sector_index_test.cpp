#include <gtest/gtest.h>
#include "core/sector_index.hpp"

TEST(SectorIndexTest, BasicTwoSectors) {
    // Setup: 8 assets, 2 sectors
    // Assets 0,2,4,6 are Tech (sector 0)
    // Assets 1,3,5,7 are Bank (sector 1)
    std::vector<uint32_t> assignments = {0, 1, 0, 1, 0, 1, 0, 1};
    
    SectorIndex index(8, 2, assignments);
    
    // Test 1: Sector boundaries
    EXPECT_EQ(index.sector_start(0), 0);
    EXPECT_EQ(index.sector_end(0), 4);
    EXPECT_EQ(index.sector_start(1), 4);
    EXPECT_EQ(index.sector_end(1), 8);
    
    // Test 2: Tech assets (0,2,4,6) should map to sorted indices 0-3
    // Test 3: Bank assets (1,3,5,7) should map to sorted indices 4-7    
    EXPECT_EQ(index.sorted_index(0), 0);
    EXPECT_EQ(index.sorted_index(1), 4);
    EXPECT_EQ(index.sorted_index(2), 1);
    EXPECT_EQ(index.sorted_index(3), 5);    
    EXPECT_EQ(index.sorted_index(4), 2);
    EXPECT_EQ(index.sorted_index(5), 6);
    EXPECT_EQ(index.sorted_index(6), 3);
    EXPECT_EQ(index.sorted_index(7), 7);  
    
    // Test 4: Verify contiguity - all Tech sorted indices should be in [0,4)
    for(auto i=index.sector_start(0); i < 8; ++i) {
        if (assignments[i] != 0) continue;
        EXPECT_GE(index.sorted_index(i), 0u);
        EXPECT_LT(index.sorted_index(i), 4u);
    }
}