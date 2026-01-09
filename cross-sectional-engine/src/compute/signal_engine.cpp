#include "compute/signal_engine.hpp"
#include "math/avx_math.hpp"
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
        total_notional = Math::avx_dot_product(
            store_.get_prices(), 
            store_.get_volumes(), 
            store_.capacity()
        );
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
    return Math::avx_mean(snapshot_prices_.data(), snapshot_size_);
}

float SignalEngine::std_dev_internal(float mean_price) const {
    return Math::avx_std_dev(snapshot_prices_.data(), snapshot_size_, mean_price);
}

void SignalEngine::zscores_internal(float* out_scores) const {
    if (snapshot_size_ == 0) {
        return;
    }

    float mean = mean_internal();
    float std_dev = std_dev_internal(mean);
    Math::avx_zscore(snapshot_prices_.data(), snapshot_size_, mean, std_dev, out_scores);
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