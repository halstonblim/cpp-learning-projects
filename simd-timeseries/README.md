# SIMD Time Series Engine

A high-performance C++ library for calculating statistical metrics on time-series data, optimized using **AVX2 Intrinsics**.

This project demonstrates a **4x speedup** over standard scalar C++ by utilizing Data-Oriented Design (SoA), memory alignment, and explicit CPU vectorization.

## Table of Contents

- [Performance Results](#performance-results)
- [Key Features](#key-features)
- [Requirements](#requirements)
- [Building & Running](#building--running)
- [Technical Details](#technical-details)

## Performance Results

Benchmarks comparing standard `std::vector` accumulation vs. AVX2 `_mm256_add_pd` intrinsics.

![Benchmark Results](benchmark_data/benchmark_results.png)

| Elements   | Scalar Time (ns) | AVX2 Time (ns) | Speedup   |
|------------|------------------|----------------|-----------|
| 8,192      | 8,184            | 2,018          | **4.06x** |
| 32,768     | 33,021           | 8,546          | **3.86x** |
| 262,144    | 269,806          | 66,844         | **4.04x** |
| 2,097,152  | 2,452,980        | 856,224        | **2.86x** |
| 8,388,608  | 9,941,260        | 4,354,170      | **2.28x** |

> **Note:** As data size exceeds L3 cache (~8MB+), speedup decreases from the theoretical max (4x) as the bottleneck shifts from CPU compute to memory bandwidth.

## Key Features

* **AVX2 Vectorization:** Processes 4 `double` values per CPU cycle via `immintrin.h`
* **Aligned Allocation:** Custom `AlignedAllocator<T>` ensures 32-byte alignment for `_mm256_load_pd`
* **Data-Oriented Design:** Struct-of-Arrays (SoA) layout maximizes cache locality

## Requirements

* C++20 compiler
* CPU with AVX2 support (Intel Haswell+, AMD Ryzen)
* CMake 3.20+

## Building & Running

```bash
# Build
mkdir build && cd build
cmake .. && make

# Run tests
./tests/simd_test
./tests/alignment_test

# Run benchmarks (from build directory)
./benchmarks/stats_benchmark --benchmark_out=../benchmark_data/results.csv --benchmark_out_format=csv

# Generate visualization (from project root)
cd ..
python3 scripts/visualize_benchmark.py benchmark_data/results.csv benchmark_data/benchmark_results.png
```

## Technical Details

### Alignment Challenge

AVX load instructions require 32-byte aligned memory addresses. Standard `new` and `std::vector` don't guarantee this, so I implemented a custom STL-compliant allocator:

```cpp
if (posix_memalign(&ptr, 32, n * sizeof(T)) != 0) {
    throw std::bad_alloc();
}
```

### Tail Handling

SIMD processes data in chunks of 4. For arrays not divisible by 4, the engine uses vectorized operations for the bulk, then a scalar loop for the remaining `size % 4` elements.
