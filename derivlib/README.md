# derivlib

A library for derivatives pricing using Monte Carlo simulation.

## Features

- **Monte Carlo Engine** with antithetic variance reduction
- **Black-Scholes Model** for geometric Brownian motion
- **European Options** (calls and puts)
- **Extensible** interface for custom models and payoffs
- **Online Statistics** using Welford's algorithm

## Requirements

- C++17 or later
- CMake 3.20+
- Google Test (automatically fetched)

## Building

```bash
cd derivlib
mkdir build && cd build
cmake ..
make
ctest  # run tests
```

## Quick Start

```cpp
#include "mc/MonteCarloEngine.h"
#include "mc/models/BlackScholesModel.h"
#include "mc/payoffs/EuropeanPayoff.h"

// Setup
double spot = 100.0, strike = 105.0, rate = 0.05, vol = 0.2, expiry = 1.0;
auto model = std::make_unique<derivlib::mc::models::BlackScholesModel>(spot, rate, vol);
derivlib::mc::MonteCarloEngine engine(std::move(model), 100000, 252);

// Price a European call
derivlib::mc::payoffs::EuropeanCall call(strike);
auto result = engine.price(call, expiry, rate);

std::cout << "Price: " << result.price << " ± " << result.std_error << std::endl;
```

## Variance Reduction

Use antithetic variates for improved convergence:

```cpp
auto result = engine.price_antithetic(call, expiry, rate);
// Typically achieves ~√2 variance reduction
```

## Custom Payoffs

Implement any payoff as a functor:

```cpp
class AsianCall {
public:
    explicit AsianCall(double strike) : strike_(strike) {}
    double operator()(const std::vector<double>& path) const {
        double avg = std::accumulate(path.begin(), path.end(), 0.0) / path.size();
        return std::max(avg - strike_, 0.0);
    }
private:
    double strike_;
};
```

## Custom Models

Extend with custom stochastic processes:

```cpp
class MyModel : public derivlib::mc::models::IProcessModel {
public:
    double generate_path(double expiry, 
                        std::vector<double>& path,
                        const std::vector<double>& random_increments) const override {
        // Your path generation logic
    }
};
```

## API

### MonteCarloEngine
```cpp
MonteCarloEngine(unique_ptr<IProcessModel> model, size_t num_paths, size_t num_steps);
MCResult price(const Payoff& payoff, double expiry, double rate);
MCResult price_antithetic(const Payoff& payoff, double expiry, double rate);
```

### MCResult
```cpp
struct MCResult {
    double price;       // Option price estimate
    double std_error;   // Standard error
};
```

## Project Structure

```
derivlib/
├── include/
│   ├── mc/
│   │   ├── MonteCarloEngine.h
│   │   ├── models/          # IProcessModel, BlackScholesModel
│   │   └── payoffs/         # EuropeanPayoff
│   └── stats/               # OnlineStats
├── src/
└── tests/
```

