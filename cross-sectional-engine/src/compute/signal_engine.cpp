#include "compute/signal_engine.hpp"
#include <cmath>
#include <immintrin.h>
#include <numeric>

// =============================================================================
// Public API - Seqlock Protected
// =============================================================================

float SignalEngine::calculate_total_notional_scalar() const {    
    float total_notional{};
    uint64_t seq;
    do {
        seq = store_.seqlock().read_begin();
        total_notional = 0.0f;
        const float* prices = store_.get_prices();
        const float* volumes = store_.get_volumes();
        size_t n_assets = store_.size();
        for (size_t i = 0; i < n_assets; ++i) {
            total_notional += prices[i] * volumes[i];
        }
    } while (store_.seqlock().read_retry(seq));

    return total_notional;
}

float SignalEngine::calculate_total_notional_avx() const {
    uint64_t seq;
    float total_notional{};
    do {
        seq = store_.seqlock().read_begin();
        __m256 total_vec = _mm256_setzero_ps();

        const float* prices = store_.get_prices();
        const float* volumes = store_.get_volumes();

        size_t i = 0;
        size_t n_assets = store_.size();
        for(; i + 8 <= n_assets; i +=8) {
            __m256 price_vec = _mm256_load_ps(&prices[i]);
            __m256 volume_vec = _mm256_load_ps(&volumes[i]);
            total_vec = _mm256_fmadd_ps(volume_vec, price_vec, total_vec);
        }

        float temp[8];
        _mm256_storeu_ps(temp, total_vec); 
        total_notional = std::accumulate(temp, temp + 8, 0.0f);

        for(; i < n_assets; ++i) {
            total_notional += prices[i] * volumes[i];
        }    
    } while (store_.seqlock().read_retry(seq));

    return total_notional;
}

float SignalEngine::calculate_mean_avx() const {
    size_t n_assets = store_.size();
    if (n_assets == 0) {
        return 0.0f;
    }
    
    float mean{};
    uint64_t seq;
    do {
        seq = store_.seqlock().read_begin();
        mean = mean_internal();
    } while (store_.seqlock().read_retry(seq));

    return mean;
}

float SignalEngine::calculate_std_dev_avx(float mean_price) const {
    size_t n_assets = store_.size();
    if (n_assets == 0) {
        return 0.0f;
    }
    
    float std_dev{};
    uint64_t seq;
    do {
        seq = store_.seqlock().read_begin();
        std_dev = std_dev_internal(mean_price);
    } while (store_.seqlock().read_retry(seq));

    return std_dev;
}

void SignalEngine::calculate_zscores_avx(float* out_scores) const {
    size_t n_assets = store_.size();
    if (n_assets == 0) {
        return;
    } 
    
    uint64_t seq;
    do {
        seq = store_.seqlock().read_begin();
        // All three operations use the same snapshot - no nested seqlocks!
        zscores_internal(out_scores);
    } while (store_.seqlock().read_retry(seq));
}

// =============================================================================
// Internal Implementations - NO seqlock, caller must hold lock
// =============================================================================

float SignalEngine::mean_internal() const {
    size_t n_assets = store_.size();
    if (n_assets == 0) {
        return 0.0f;
    }

    __m256 total_vec = _mm256_setzero_ps();
    const float* prices = store_.get_prices();

    size_t i = 0;
    for (; i + 8 <= n_assets; i += 8) {
        __m256 price_vec = _mm256_load_ps(&prices[i]);
        total_vec = _mm256_add_ps(price_vec, total_vec);
    }

    float temp[8];
    _mm256_storeu_ps(temp, total_vec); 
    float total = std::accumulate(temp, temp + 8, 0.0f);

    for (; i < n_assets; ++i) {
        total += prices[i];
    }

    return total / static_cast<float>(n_assets);
}

float SignalEngine::std_dev_internal(float mean_price) const {
    size_t n_assets = store_.size();
    if (n_assets == 0) {
        return 0.0f;
    }

    __m256 mean_vec = _mm256_set1_ps(mean_price);
    __m256 variance_vec = _mm256_setzero_ps();

    const float* prices = store_.get_prices();
    size_t i = 0;
    for (; i + 8 <= n_assets; i += 8) {
        __m256 price_vec = _mm256_load_ps(&prices[i]);
        __m256 diff_vec = _mm256_sub_ps(price_vec, mean_vec);
        __m256 diffsq_vec = _mm256_mul_ps(diff_vec, diff_vec);
        variance_vec = _mm256_add_ps(diffsq_vec, variance_vec);
    }

    float temp[8];
    _mm256_storeu_ps(temp, variance_vec); 
    float variance = std::accumulate(temp, temp + 8, 0.0f);

    for (; i < n_assets; ++i) {
        variance += (prices[i] - mean_price) * (prices[i] - mean_price);
    }

    return std::sqrt(variance / static_cast<float>(n_assets));
}

void SignalEngine::zscores_internal(float* out_scores) const {
    size_t n_assets = store_.size();
    if (n_assets == 0) {
        return;
    }

    // Compute mean and std_dev on the SAME snapshot (no seqlock here)
    float mean = mean_internal();
    float std_dev = std_dev_internal(mean);
    
    std::fill(out_scores, out_scores + n_assets, 0.0f);
    if (std_dev == 0.0f) {
        return;
    }

    float inv_std_dev = 1.0f / std_dev;
    __m256 mean_vec = _mm256_set1_ps(mean);
    __m256 inv_std_vec = _mm256_set1_ps(inv_std_dev);
    const float* prices = store_.get_prices();
    
    size_t i = 0;
    for (; i + 8 <= n_assets; i += 8) {
        __m256 price_vec = _mm256_load_ps(&prices[i]);
        __m256 diff_vec = _mm256_sub_ps(price_vec, mean_vec);
        __m256 zscore_vec = _mm256_mul_ps(diff_vec, inv_std_vec);
        _mm256_storeu_ps(&out_scores[i], zscore_vec);
    }
    for (; i < n_assets; ++i) {
        out_scores[i] = (prices[i] - mean) * inv_std_dev;
    }
}
