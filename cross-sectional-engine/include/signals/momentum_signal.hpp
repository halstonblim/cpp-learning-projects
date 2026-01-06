#pragma once
#include "signals/signal_interface.hpp"
#include "math/avx_math.hpp"

struct MomentumSignal {
    static void calculate(const MarketSnapshot& snap, float* output) {
        if (snap.size == 0) return;
        Math::avx_mult(snap.prices, snap.volumes, output, snap.size);
    }
};