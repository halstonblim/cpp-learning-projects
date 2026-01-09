#pragma once
#include "math/avx_math.hpp"

struct PnLCalculator {
    // Compute returns from price series
    // Note: All arrays must be 32-byte aligned for AVX operations
    static void calculate_returns(const float* prev_prices, 
        const float* curr_prices, 
        float* returns, 
        size_t n) {
            Math::avx_return(curr_prices, prev_prices, returns, n);
        }
    
    // Compute total PnL: sum(signals * returns)
    [[nodiscard]] static float calculate_pnl(const float* signals, const float* returns, size_t n) {
        return Math::avx_dot_product(signals, returns, n);
    }
};