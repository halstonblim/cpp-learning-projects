# Online Statistics

A C++ header-only library for computing statistics (mean and variance) incrementally using Welford's algorithm. Supports any numeric type and includes comprehensive tests with GoogleTest.

## Building

This project uses CMake and includes GoogleTest for testing.

```bash
mkdir build
cd build
cmake ..
make
```

## Running Tests

```bash
cd build
ctest
```

## Usage

```cpp
#include <welford.h>

// Create an OnlineStats instance
OnlineStats<double> stats;

// Add values incrementally
stats.Add(10.0);
stats.Add(20.0);
stats.Add(30.0);

// Get statistics
double mean = stats.Mean();        // 20.0
double variance = stats.Variance(); // 66.67
int64_t count = stats.Count();     // 3
```
