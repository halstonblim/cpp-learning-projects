#pragma once
#include "math/avx_math.hpp"

struct PnLCalculator {
    static void calculate_returns(const float* prev_prices, 
        const float* curr_prices, 
        float* returns, 
        size_t n) {
            Math::avx_return(curr_prices, prev_prices, returns, n);
    }
    
    [[nodiscard]] static float calculate_pnl(const float* signals, const float* returns, size_t n) {
        return Math::avx_dot_product(signals, returns, n);
    }
};

struct StrategyMetrics {
    float total_pnl;
    float sharpe_ratio;
};

inline StrategyMetrics calculate_metrics(
    const float* pnl_series,
    size_t num_periods,
    float risk_free_rate = 0.0f
) {
    float mean_pnl = Math::avx_mean(pnl_series, num_periods);
    float std_pnl = Math::avx_std_dev(pnl_series, num_periods, mean_pnl);
    float total_pnl = mean_pnl * static_cast<float>(num_periods);
    float sharpe = (std_pnl > 0.0f)
        ? (mean_pnl - risk_free_rate) / std_pnl
        : 0.0f;
    return {total_pnl, sharpe};

}