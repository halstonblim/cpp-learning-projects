#pragma once

#include <cstddef>

struct MCParams {
    double spot;
    double strike;
    double rate;
    double vol;
    double expiry; // T in years
    std::size_t num_paths;
};

struct MCResult {
    double price;
    double std_error;
};

MCResult price_european_callmc(const MCParams& parms);