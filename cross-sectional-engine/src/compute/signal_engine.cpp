#include "compute/signal_engine.hpp"
#include <cmath>
#include <immintrin.h>
#include <numeric>

float SignalEngine::calculate_total_notional_scalar() const {    
    const float* prices = store_.get_prices();
    const float* volumes = store_.get_volumes();

    float total_notional{};
    size_t n_assets = store_.size();
    for (size_t i = 0; i < n_assets; ++i) {
        total_notional += prices[i] * volumes[i];
    }
    return total_notional;
}

float SignalEngine::calculate_total_notional_avx() const {
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
    float total_notional = std::accumulate(temp, temp + 8, 0.0f);

    for(; i < n_assets; ++i) {
        total_notional += prices[i] * volumes[i];
    }    

    return total_notional;
}

float SignalEngine::calculate_mean_avx() const {
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

float SignalEngine::calculate_std_dev_avx(float mean_price) const {
    size_t n_assets = store_.size();
    if (n_assets == 0) {
        return 0.0f;
    }

    __m256 mean_vec = _mm256_set1_ps(mean_price);
    __m256 variance_vec = _mm256_setzero_ps();
    __m256 diff_vec;
    __m256 diffsq_vec;

    const float* prices = store_.get_prices();
    size_t i = 0;
    for (; i + 8 <= n_assets; i += 8) {
        __m256 price_vec = _mm256_load_ps(&prices[i]);
        diff_vec = _mm256_sub_ps(price_vec, mean_vec);
        diffsq_vec =  _mm256_mul_ps(diff_vec, diff_vec);
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

void SignalEngine::calculate_zscores_avx(float* out_scores) const {
    size_t n_assets = store_.size();
    if (n_assets == 0) {
        return;
    } 

    float mean = calculate_mean_avx();
    float std_dev = calculate_std_dev_avx(mean);
    if (std_dev == 0.0f) {
        std::fill(out_scores, out_scores + n_assets, 0.0f);
        return;
    }
    float inv_std_dev = 1.0f / std_dev;
    __m256 diff_vec;
    __m256 zscore_vec;
    __m256 mean_vec = _mm256_set1_ps(mean);
    __m256 inv_std_vec = _mm256_set1_ps(inv_std_dev);
    const float* prices = store_.get_prices();
    size_t i = 0;
    for (; i + 8 <= n_assets; i += 8) {
        __m256 price_vec = _mm256_load_ps(&prices[i]);
        diff_vec = _mm256_sub_ps(price_vec, mean_vec);
        zscore_vec =  _mm256_mul_ps(diff_vec, inv_std_vec);
        _mm256_storeu_ps(&out_scores[i], zscore_vec);
    }
    for (; i < n_assets; ++i) {
        out_scores[i] = (prices[i] - mean) * inv_std_dev;
    }
}

