#pragma once
#include "strategy/pnl_calculator.hpp"
#include "core/aligned_allocator.hpp"
#include "core/sector_index.hpp"
#include <memory>

class BacktesterSector {
public:
    // New constructor that accepts sector configuration
    BacktesterSector(size_t num_assets, size_t num_periods, size_t num_sectors);
    
    StrategyMetrics run();
    
private:
    // Existing buffers...
    std::vector<float, AlignedAllocator<float, 32>> prev_prices_;
    std::vector<float, AlignedAllocator<float, 32>> curr_prices_;
    std::vector<float, AlignedAllocator<float, 32>> returns_;
    std::vector<float, AlignedAllocator<float, 32>> zscores_;
    std::vector<float, AlignedAllocator<float, 32>> signals_;
    std::vector<float> pnl_series_;
    
    std::vector<float, AlignedAllocator<float, 32>> sorted_prices_;
    std::vector<float, AlignedAllocator<float, 32>> sorted_zscores_;
    std::unique_ptr<SectorIndex> sector_index_;
    
    size_t num_periods_;
    size_t num_assets_;
    size_t num_sectors_;
    
    // NEW: Helper methods
    void sort_to_sector_order(const float* src, float* dst);
    void unsort_from_sector_order(const float* src, float* dst);
};