#pragma once
#include <cstddef>
#include <concepts>

namespace Signal {
    constexpr float Sell = -1.0f;
    constexpr float Hold =  0.0f;
    constexpr float Buy  =  1.0f;
}

template<typename T>
concept TradingStrategy = requires(const typename T::Inputs& inputs, float* output, size_t n) {
    typename T::Inputs;
    { T::calculate(inputs, output, n) } -> std::same_as<void>;
};