#pragma once
#include <cstddef>
#include <concepts>

struct MarketSnapshot {
    const float* prices;
    const float* volumes;
    size_t size;
};

template<typename T>
concept SignalStrategy = requires(const MarketSnapshot& snap, float* output) {
    { T::calculate(snap, output) } -> std::same_as<void>;
};
