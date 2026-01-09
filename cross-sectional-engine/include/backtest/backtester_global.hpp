#pragma once
#include "strategy/pnl_calculator.hpp"
#include "core/aligned_allocator.hpp"

class BacktesterGlobal {
public:
    BacktesterGlobal(size_t num_assets, size_t num_periods);
    StrategyMetrics run();
    
private:
    std::vector<float, AlignedAllocator<float, 32>> prev_prices_;
    std::vector<float, AlignedAllocator<float, 32>> curr_prices_;
    std::vector<float, AlignedAllocator<float, 32>> returns_;
    std::vector<float, AlignedAllocator<float, 32>> zscores_;
    std::vector<float, AlignedAllocator<float, 32>> signals_;
    std::vector<float> pnl_series_;
    size_t num_periods_;
    size_t num_assets_;
};