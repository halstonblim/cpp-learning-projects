#pragma once
#include <cstddef>
#include <concepts>

struct MarketSnapshot {
    const float* prices;   // Points to snapshot_prices_.data()
    const float* volumes;  // Points to snapshot_volumes_.data()
    size_t size;           // = snapshot_size_
};

// C++20 Concept to enforce the Signal interface at compile time.
// Every signal must have a static 'calculate' method.
template<typename T>
concept SignalStrategy = requires(const MarketSnapshot& snap, float* output) {
    { T::calculate(snap, output) } -> std::same_as<void>;
};

