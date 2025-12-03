#include "compute/signal_engine.hpp"
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