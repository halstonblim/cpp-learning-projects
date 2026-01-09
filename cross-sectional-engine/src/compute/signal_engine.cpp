#include "compute/signal_engine.hpp"
#include <cmath>
#include <immintrin.h>
#include <numeric>
#include <cstring>

float SignalEngine::calculate_total_notional_scalar() const {    
    float total_notional{};
    uint64_t seq;
    do {
        seq = store_.seqlock().read_begin();
        total_notional = 0.0f;
        const float* prices = store_.get_prices();
        const float* volumes = store_.get_volumes();
        size_t n_assets = store_.capacity();
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
        size_t n_assets = store_.capacity();
        for (; i + 8 <= n_assets; i += 8) {
            __m256 price_vec = _mm256_load_ps(&prices[i]);
            __m256 volume_vec = _mm256_load_ps(&volumes[i]);
            total_vec = _mm256_fmadd_ps(volume_vec, price_vec, total_vec);
        }

        float temp[8];
        _mm256_storeu_ps(temp, total_vec); 
        total_notional = std::accumulate(temp, temp + 8, 0.0f);

        for (; i < n_assets; ++i) {
            total_notional += prices[i] * volumes[i];
        }    
    } while (store_.seqlock().read_retry(seq));

    return total_notional;
}

PriceStats SignalEngine::calculate_stats_avx() const {
    size_t n_assets = store_.capacity();
    if (n_assets == 0) {
        return PriceStats{};
    }
    snapshot();
    float mean = mean_internal();
    float std_dev = std_dev_internal(mean);
    return PriceStats{mean, std_dev};
}

float SignalEngine::calculate_mean_avx() const {
    size_t n_assets = store_.capacity();
    if (n_assets == 0) {
        return 0.0f;
    }
    snapshot();
    return mean_internal();
}

void SignalEngine::calculate_zscores_avx(float* out_scores) const {
    size_t n_assets = store_.capacity();
    if (n_assets == 0) {
        return;
    } 
    snapshot();
    zscores_internal(out_scores);
}

float SignalEngine::mean_internal() const {
    if (snapshot_size_ == 0) {
        return 0.0f;
    }

    __m256 total_vec = _mm256_setzero_ps();
    const float* prices = snapshot_prices_.data();

    size_t i = 0;
    for (; i + 8 <= snapshot_size_; i += 8) {
        __m256 price_vec = _mm256_load_ps(&prices[i]);
        total_vec = _mm256_add_ps(price_vec, total_vec);
    }

    float temp[8];
    _mm256_storeu_ps(temp, total_vec); 
    float total = std::accumulate(temp, temp + 8, 0.0f);

    for (; i < snapshot_size_; ++i) {
        total += prices[i];
    }

    return total / static_cast<float>(snapshot_size_);
}

float SignalEngine::std_dev_internal(float mean_price) const {
    if (snapshot_size_ == 0) {
        return 0.0f;
    }

    __m256 mean_vec = _mm256_set1_ps(mean_price);
    __m256 variance_vec = _mm256_setzero_ps();

    const float* prices = snapshot_prices_.data();
    size_t i = 0;
    for (; i + 8 <= snapshot_size_; i += 8) {
        __m256 price_vec = _mm256_load_ps(&prices[i]);
        __m256 diff_vec = _mm256_sub_ps(price_vec, mean_vec);
        __m256 diffsq_vec = _mm256_mul_ps(diff_vec, diff_vec);
        variance_vec = _mm256_add_ps(diffsq_vec, variance_vec);
    }

    float temp[8];
    _mm256_storeu_ps(temp, variance_vec); 
    float variance = std::accumulate(temp, temp + 8, 0.0f);

    for (; i < snapshot_size_; ++i) {
        variance += (prices[i] - mean_price) * (prices[i] - mean_price);
    }

    return std::sqrt(variance / static_cast<float>(snapshot_size_));
}

void SignalEngine::zscores_internal(float* out_scores) const {
    if (snapshot_size_ == 0) {
        return;
    }

    float mean = mean_internal();
    float std_dev = std_dev_internal(mean);
    
    std::fill(out_scores, out_scores + snapshot_size_, 0.0f);
    if (std_dev == 0.0f) {
        return;
    }

    float inv_std_dev = 1.0f / std_dev;
    __m256 mean_vec = _mm256_set1_ps(mean);
    __m256 inv_std_vec = _mm256_set1_ps(inv_std_dev);
    const float* prices = snapshot_prices_.data();
    
    size_t i = 0;
    for (; i + 8 <= snapshot_size_; i += 8) {
        __m256 price_vec = _mm256_load_ps(&prices[i]);
        __m256 diff_vec = _mm256_sub_ps(price_vec, mean_vec);
        __m256 zscore_vec = _mm256_mul_ps(diff_vec, inv_std_vec);
        _mm256_storeu_ps(&out_scores[i], zscore_vec);
    }
    for (; i < snapshot_size_; ++i) {
        out_scores[i] = (prices[i] - mean) * inv_std_dev;
    }
}

void SignalEngine::snapshot() const {
    size_t cap = store_.capacity();
    if (snapshot_prices_.size() < cap) {
        snapshot_prices_.resize(cap);
        snapshot_volumes_.resize(cap);
    }

    uint64_t seq;
    do {
        seq = store_.seqlock().read_begin();
        size_t n_assets = store_.capacity();
        std::memcpy(snapshot_prices_.data(), store_.get_prices(), n_assets * sizeof(float));
        std::memcpy(snapshot_volumes_.data(), store_.get_volumes(), n_assets * sizeof(float));
        snapshot_size_ = n_assets;
    } while (store_.seqlock().read_retry(seq));
}