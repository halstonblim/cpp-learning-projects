#pragma once
#include <immintrin.h>
#include <numeric>
#include <cstddef>

namespace Math {

[[nodiscard]] inline float avx_mean(const float* data, size_t size) {
    if (size == 0) return 0.0f;

    __m256 total_vec = _mm256_setzero_ps();
    size_t i = 0;

    for(; i + 8 <= size; i += 8) {
        __m256 vec = _mm256_load_ps(&data[i]);
        total_vec = _mm256_add_ps(vec, total_vec);
    }

    float temp[8];
    _mm256_storeu_ps(temp, total_vec);
    float total = std::accumulate(temp, temp + 8, 0.0f);

    for (; i < size; ++i) {
        total += data[i];
    }

    return total / static_cast<float>(size);
}

[[nodiscard]] inline float avx_std_dev(const float* data, size_t size, float mean) {
    if (size == 0) return 0.0f;

    __m256 mean_vec = _mm256_set1_ps(mean);
    __m256 var_vec = _mm256_setzero_ps();
    size_t i = 0;

    for (; i + 8 <= size; i += 8) {
        __m256 val = _mm256_load_ps(&data[i]);
        __m256 diff = _mm256_sub_ps(val, mean_vec);
        __m256 diff_sq = _mm256_mul_ps(diff, diff);
        var_vec = _mm256_add_ps(diff_sq, var_vec);
    }

    float temp[8];
    _mm256_storeu_ps(temp, var_vec);
    float total = std::accumulate(temp, temp + 8, 0.0f);

    for (; i < size; ++i) {
        total += (data[i] - mean) * (data[i] - mean);
    }

    return total / static_cast<float>(size);
}

} // namspace Math