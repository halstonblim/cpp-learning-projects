#pragma once
#include <immintrin.h> // AVX intrinsics
#include <cstddef>     // size_t

/**
 * Calculates sum of an array using AVX2 intrinsics.
 * Constraint: 'data' MUST be aligned to 32-byte boundaries.
 */
inline double sum_avx2(const double* data, size_t size) {
    // 1. Initialize accumulator vector to [0.0, 0.0, 0.0, 0.0]
    __m256d vec_sum = _mm256_setzero_pd();

    size_t i = 0;

    // 2. Main SIMD Loop: Process 4 doubles per cycle
    // We stop when we have fewer than 4 elements left.
    for (; i + 4 <= size; i += 4) {
        // Load 4 doubles from aligned memory
        __m256d vec_data = _mm256_load_pd(&data[i]);
        
        // Accumulate: vec_sum[k] += vec_data[k]
        vec_sum = _mm256_add_pd(vec_sum, vec_data);
    }

    // 3. Horizontal Sum: Reduce vector [v0, v1, v2, v3] to scalar (v0+v1+v2+v3)
    // We dump the register back to a temporary array to sum it up.
    // (There are faster ways using shuffle intrinsics, but this is most readable).
    double temp[4];
    _mm256_storeu_pd(temp, vec_sum); 
    double total_sum = temp[0] + temp[1] + temp[2] + temp[3];

    // 4. Tail Loop: Process remaining elements (0 to 3 items)
    for (; i < size; ++i) {
        total_sum += data[i];
    }

    return total_sum;
}