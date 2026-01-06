#pragma once
#include "signals/signal_interface.hpp"
#include "math/avx_math.hpp"
#include <immintrin.h>
#include <algorithm>

struct ZScoreSignal {
    static void calculate(const MarketSnapshot& snap, float* output) {
        if (snap.size == 0) return;

        // 1. Calculate Statistics using our Math primitives
        float mean = Math::avx_mean(snap.prices, snap.size);
        float std_dev = Math::avx_std_dev(snap.prices, snap.size, mean);

        // Avoid division by zero
        if (std_dev == 0.0f) {
            std::fill(output, output + snap.size, 0.0f);
            return;
        }

        float inv_std_dev = 1.0f / std_dev;

        // 2. Vectorized Z-Score Calculation: (Price - Mean) * InvStdDev
        __m256 mean_vec = _mm256_set1_ps(mean);
        __m256 inv_std_vec = _mm256_set1_ps(inv_std_dev);

        size_t i = 0;
        for (; i + 8 <= snap.size; i += 8) {
            __m256 price_vec = _mm256_load_ps(&snap.prices[i]);
            __m256 diff_vec = _mm256_sub_ps(price_vec, mean_vec);
            __m256 zscore_vec = _mm256_mul_ps(diff_vec, inv_std_vec);
            
            // Store result directly to output buffer
            _mm256_storeu_ps(&output[i], zscore_vec);
        }

        // Tail handling
        for (; i < snap.size; ++i) {
            output[i] = (snap.prices[i] - mean) * inv_std_dev;
        }
    }
};