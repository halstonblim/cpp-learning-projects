#pragma once
#include "signals/signal_interface.hpp"

// Executes multiple signals in sequence via fold expression
template<SignalStrategy... Signals>
class SignalPipeline {
public:
    static void execute(const MarketSnapshot& snap, float** outputs) {
        int index = 0;
        (..., (Signals::calculate(snap, outputs[index++])));
    }
};