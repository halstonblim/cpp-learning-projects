#pragma once
#include "signals/signal_interface.hpp"

// Variadic Template: Takes a list of Signal types (strategies)
template<SignalStrategy... Signals>
class SignalPipeline {
public:
    // Execute all signals in the pipeline sequentially
    // outputs: A generic container (like a vector of pointers) where each signal writes its results
    static void execute(const MarketSnapshot& snap, float** outputs) {
        int index = 0;
        
        // Fold expression expands to
        // SignalA::calculate(snap, outputs[0]);
        // SignalB::calculate(snap, outputs[1]);
        // ...
        (..., (
            Signals::calculate(snap, outputs[index++])
        ));
    }
};