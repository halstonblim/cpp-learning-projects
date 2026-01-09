#pragma once
#include <immintrin.h>
#include <numeric>
#include <cstddef>

namespace Math {

inline void avx_return(const float* current, const float* previous, float* result, size_t size) {
    if (size == 0) return;

    __m256 diff_vec = _mm256_setzero_ps();
    __m256 return_vec = _mm256_setzero_ps();
    size_t i = 0;

    for(; i + 8 <= size; i += 8) {
        __m256 curr = _mm256_load_ps(&current[i]);
        __m256 prev = _mm256_load_ps(&previous[i]);
        diff_vec = _mm256_sub_ps(curr, prev);
        return_vec = _mm256_div_ps(diff_vec,prev);
        _mm256_store_ps(&result[i], return_vec);
    }

    for (; i < size; ++i) {
        result[i] = (current[i] - previous[i]) / previous[i];
    }
}

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

    return std::sqrt(total / static_cast<float>(size));
}

inline void avx_mult(const float* data1, const float* data2, float* result, size_t size) {
    if (size == 0) return;

    size_t i = 0;

    // Process 8 floats at a time using AVX
    for (; i + 8 <= size; i += 8) {
        __m256 vec1 = _mm256_load_ps(&data1[i]);
        __m256 vec2 = _mm256_load_ps(&data2[i]);
        __m256 product = _mm256_mul_ps(vec1, vec2);
        _mm256_store_ps(&result[i], product);
    }

    // Handle remaining elements
    for (; i < size; ++i) {
        result[i] = data1[i] * data2[i];
    }
}

[[nodiscard]] inline float avx_dot_product(const float* data1, const float* data2, size_t size) {
    if (size == 0) return 0.0f;

    __m256 sum_vec = _mm256_setzero_ps();
    size_t i = 0;

    for (; i + 8 <= size; i += 8) {
        __m256 vec1 = _mm256_load_ps(&data1[i]);
        __m256 vec2 = _mm256_load_ps(&data2[i]);
        sum_vec = _mm256_fmadd_ps(vec1, vec2, sum_vec);  // fused multiply-add
    }

    float temp[8];
    _mm256_storeu_ps(temp, sum_vec);
    float total = std::accumulate(temp, temp + 8, 0.0f);

    for (; i < size; ++i) {
        total += data1[i] * data2[i];
    }

    return total;
}

} // namspace Math