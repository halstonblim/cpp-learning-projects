#pragma once
#include "signals/signal_interface.hpp"
#include "math/avx_math.hpp"

struct ZScoreSignal {
    static void calculate(const MarketSnapshot& snap, float* output) {
        if (snap.size == 0) return;
        float mean = Math::avx_mean(snap.prices, snap.size);
        float std_dev = Math::avx_std_dev(snap.prices, snap.size, mean);
        Math::avx_zscore(snap.prices, snap.size, mean, std_dev, output);
    }
};