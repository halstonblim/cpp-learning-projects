#pragma once
#include "strategy/strategy_interface.hpp"
#include <immintrin.h>

struct ZScoreStrategy {
    struct Inputs {
        const float* zscores;
    };
    
    static void calculate(const Inputs& inputs, float* output, size_t n) {
        __m256 upper = _mm256_set1_ps(2.0f);
        __m256 lower = _mm256_set1_ps(-2.0f);
        __m256 sell_val = _mm256_set1_ps(Signal::Sell);
        __m256 buy_val  = _mm256_set1_ps(Signal::Buy);
        __m256 hold_val = _mm256_setzero_ps();
    
        size_t i = 0;
        for (; i + 8 <= n; i += 8) { 
            __m256 zscores_vec = _mm256_loadu_ps(&inputs.zscores[i]);
    
            __m256 sell_mask = _mm256_cmp_ps(zscores_vec, upper, _CMP_GT_OQ);
            __m256 buy_mask = _mm256_cmp_ps(zscores_vec, lower, _CMP_LT_OQ);
    
            __m256 result = hold_val;
            result = _mm256_blendv_ps(result, sell_val, sell_mask);
            result = _mm256_blendv_ps(result, buy_val, buy_mask);
            
            _mm256_storeu_ps(&output[i], result);         
        }
        for (; i < n; ++i) {
            if (inputs.zscores[i] > 2.0f) output[i] = Signal::Sell;
            else if (inputs.zscores[i] < -2.0f) output[i] = Signal::Buy;
            else output[i] = Signal::Hold;
        }                
    }
};