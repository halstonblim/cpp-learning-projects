#pragma once
#include "core/universe_store.hpp"

class SignalEngine {
public:
    SignalEngine(UniverseStore& store) : store_(store) {}

    [[nodiscard]] float calculate_total_notional_scalar() const;

private:
    UniverseStore& store_;

};