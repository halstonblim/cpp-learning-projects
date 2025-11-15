# Monte Carlo Option Pricer

A C++ implementation of a Monte Carlo simulation for pricing European call options.
## Features

- Monte Carlo simulation for European call option pricing
- Computes call price by averaging monte carlo paths via Welford's algorithm for mean and standard error
- Unit tests with Google Test framework comparing Monte Carlo against Black Scholes formula

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Running Tests

```bash
cd build
ctest
# or run the test executable directly
./tests/test_mc_option_pricer
```

## Usage

```cpp
#include "mc_option_pricer.h"

MCOptionPricer::MCParams params{
    .spot = 100.0,      // Current stock price
    .strike = 105.0,    // Strike price
    .rate = 0.05,       // Risk-free rate
    .vol = 0.2,         // Volatility
    .expiry = 1.0,      // Time to expiry (years)
    .num_paths = 100000 // Number of simulation paths
};

auto result = MCOptionPricer::price_european_call_mc(params);
// result.price contains the option price
// result.std_error contains the standard error
```

## Project Structure

- `include/` - Header files
- `src/` - Implementation files
- `tests/` - Unit tests
- `build/` - Build artifacts (generated)
