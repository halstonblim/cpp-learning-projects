# C++ Learning Projects

A collection of high-performance computing and quantitative finance projects using modern C++ (17/20).

## Project Overview

| Category | Project | Description |
| :--- | :--- | :--- |
| **Market Microstructure** | **[Limit Order Book](./limit-order-book)** | Core matching engine supporting partial fills and resting liquidity. |
| | **[Market Data Processor](./market-data-processor)** | Low-latency benchmarking of Lock-Free vs. Lock-Based queues. |
| **HPC** | **[SIMD Time Series](./simd-timeseries)** | Statistical engine optimized with AVX2 intrinsics (4x speedup). |
| | **[Online Statistics](./online-stats)** | Header-only library for memory-efficient running statistics using Welford's algorithm. |
| **Quant Finance** | **[DerivLib](./derivlib)** | Extensible Monte Carlo library for derivatives pricing with variance reduction. |
| | **[MC Option Pricer](./mc-option-pricer)** | Standalone European Call option pricer validating Black-Scholes. |

## Quick Build

All projects use **CMake**. To build any project:

```bash
cd <project_name>
mkdir build && cd build
cmake .. && cmake --build .
