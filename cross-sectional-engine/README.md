# Cross-Sectional Signal Engine

A high-performance C++20 backtesting engine for cross-sectional equity strategies, optimized with AVX2 SIMD instructions.

## Table of Contents

- [Overview](#overview)
- [Components](#components)
- [Building](#building)
- [Running](#running)
- [Tests](#tests)
- [Benchmarks](#benchmarks)
- [Architecture](#architecture)
  - [Market Feed (Live Simulation)](#market-feed-live-simulation)
  - [Backtester (Historical Simulation)](#backtester-historical-simulation)
  - [Shared Infrastructure](#shared-infrastructure)

## Overview

This engine computes statistical signals (z-scores, momentum) across a universe of assets and simulates trading strategies. The architecture prioritizes low-latency execution through:

- **SoA (Structure of Arrays)** data layout for cache-efficient SIMD operations
- **Seqlock** for lock-free concurrent reads during signal computation
- **SPSC ring buffer** for market data ingestion
- **32-byte aligned memory** for AVX2 vectorization

## Components

### Core
- `UniverseStore` - SoA storage for prices, volumes, bids, asks with seqlock protection
- `Seqlock` - Single-writer, multi-reader synchronization primitive
- `LockFreeQueue` - SPSC ring buffer for market updates
- `SectorIndex` - Maps assets to sectors for sector-neutral computations
- `AlignedAllocator` - Custom allocator ensuring 32-byte alignment

### Signal Computation
- `SignalEngine` - Computes cross-sectional statistics (mean, stddev, z-scores) using AVX2
- `ZScoreSignal` - Global z-score normalization
- `SectorNeutralSignal` - Z-scores relative to sector mean
- `MomentumSignal` - Price-volume momentum indicator
- `SignalPipeline` - Variadic template for chaining signals

### Strategy
- `ZScoreStrategy` - Mean reversion strategy: buy z < -2, sell z > 2
- `PnLCalculator` - AVX-accelerated returns and P&L computation

### Backtesting
- `BacktesterGlobal` - Runs strategy against entire universe
- `BacktesterSector` - Runs sector-neutral strategy

### Ingestion
- `MarketFeed` - Simulated market data producer/consumer with queue metrics

## Building

```bash
mkdir build && cd build
cmake ..
make -j4
```

Requires: CMake 3.20+, C++20 compiler with AVX2/FMA support.

## Running

```bash
# Global backtest
./bin/backtest_global

# Sector-neutral backtest  
./bin/backtest_sector

# Market feed throughput test
./bin/market_feed_test
```

## Tests

```bash
ctest --output-on-failure
```

## Benchmarks

```bash
./benchmarks/signal_engine_benchmark
./benchmarks/aos_vs_soa_benchmark
./benchmarks/pipeline_benchmark
```

## Architecture

The engine supports two operating modes that share common infrastructure:

| Mode | Purpose | Data Source | Concurrency | Use Case |
|------|---------|-------------|-------------|----------|
| **Market Feed** | Live simulation | Producer thread generates ticks | Multi-threaded (producer/consumer/readers) | Throughput testing, live trading prototype |
| **Backtester** | Historical simulation | Synthetic price series in memory | Single-threaded loop | Strategy research, P&L analysis |

Both modes use the same AVX2 math primitives and signal computation logic, but differ in how data flows through the system.

### Market Feed (Live Simulation)

```
+------------------+      +-------------------+      +------------------+
|  Producer Thread |      |  LockFreeQueue    |      |  Consumer Thread |
|                  |      |   (SPSC Ring)     |      |                  |
|  - Random price  | push |                   | pop  |  - write_lock()  |
|    generation    |----->|  [update][update] |----->|  - update_tick() |
|  - GBM-like      |      |  [......][......] |      |  - write_unlock()|
|    simulation    |      |                   |      |                  |
+------------------+      +-------------------+      +--------+---------+
                                                              |
                                                              v
                          +-----------------------------------+
                          |          UniverseStore (SoA)      |
                          |  +-------+-------+------+------+  |
                          |  |prices |volumes| bids | asks |  |
                          |  |[====>]|[====>]|[===>]|[===>]|  |
                          |  +-------+-------+------+------+  |
                          |           (32-byte aligned)       |
                          |              + Seqlock            |
                          +-----------------------------------+
```

The producer simulates market ticks and pushes `MarketUpdate` structs to a lock-free queue. The consumer pops updates and writes them to SoA storage under seqlock protection. Readers (signal engines) can concurrently snapshot and compute signals via seqlock retry loop without blocking the writer.

### Backtester (Historical Simulation)

```
                         +------------------------------------------+
                         |              Backtest Loop               |
                         |           (for each period t)            |
                         +------------------------------------------+
                                            |
              +-----------------------------+-----------------------------+
              |                             |                             |
              v                             v                             v
+---------------------------+  +---------------------------+  +---------------------------+
|     Price Generation      |  |    Signal Computation     |  |     Strategy + P&L        |
|                           |  |                           |  |                           |
|  curr_prices[i] =         |  |  ZScoreSignal::calculate  |  |  ZScoreStrategy::calc     |
|    prev_prices[i] *       |  |         or                |  |    z > 2  -> SELL         |
|    (1 + noise)            |  |  SectorNeutralSignal      |  |    z < -2 -> BUY          |
|                           |  |    (sector-sorted)        |  |    else   -> HOLD         |
+---------------------------+  +---------------------------+  +---------------------------+
                                            |                             |
                                            v                             v
                               +---------------------------+  +---------------------------+
                               |   AVX2 Math Primitives    |  |     PnLCalculator         |
                               |                           |  |                           |
                               |  avx_mean()               |  |  returns = avx_return()   |
                               |  avx_std_dev()            |  |  pnl = dot(signals,       |
                               |  avx_zscore()             |  |           returns)        |
                               +---------------------------+  +---------------------------+
                                                                          |
                                                                          v
                                                              +---------------------------+
                                                              |    calculate_metrics()    |
                                                              |                           |
                                                              |  total_pnl, sharpe_ratio  |
                                                              +---------------------------+
```

**Global vs Sector Backtester:**

```
Global Backtest:                      Sector Backtest:

prices[] ---------------------->      prices[] ---> sort_to_sector_order()
    |                                                       |
    v                                                       v
ZScoreSignal (global mean/std)        SectorNeutralSignal (per-sector mean/std)
    |                                                       |
    v                                                       v
zscores[] ------------------->        sorted_zscores[] ---> unsort_from_sector_order()
                                                            |
                                                            v
                                                      zscores[]
```

Sector-neutral signals group assets by sector for contiguous AVX operations, then unsort results back to original order.

### Shared Infrastructure

Both modes reuse the same core components:

```
+------------------------------------------------------------------+
|                        Shared Components                          |
+------------------------------------------------------------------+
|                                                                    |
|  +------------------+    +------------------+    +---------------+ |
|  | AVX2 Math        |    | Signal Classes   |    | Strategy      | |
|  |  - avx_mean()    |    |  - ZScoreSignal  |    |  - ZScoreStrat| |
|  |  - avx_std_dev() |    |  - SectorNeutral |    |  - PnLCalc    | |
|  |  - avx_zscore()  |    |  - Momentum      |    |               | |
|  +------------------+    +------------------+    +---------------+ |
|                                                                    |
+------------------------------------------------------------------+
          ^                        ^                      ^
          |                        |                      |
    +-----+-----+            +-----+-----+          +-----+-----+
    |           |            |           |          |           |
+---+---+   +---+---+    +---+---+   +---+---+  +---+---+   +---+---+
| Market|   | Back- |    | Market|   | Back- |  | Market|   | Back- |
| Feed  |   | tester|    | Feed  |   | tester|  | Feed  |   | tester|
+-------+   +-------+    +-------+   +-------+  +-------+   +-------+
```

The key difference is data flow:
- **Market Feed**: Async producer -> queue -> consumer -> UniverseStore -> SignalEngine reads via seqlock
- **Backtester**: Synchronous loop generates prices directly into aligned buffers, calls signals inline
