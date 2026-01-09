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
  - [Live Trading Loop](#live-trading-loop-main_livecpp)
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

From the `build/` directory:

```bash
# Global backtest
./src/backtest_global

# Sector-neutral backtest  
./src/backtest_sector

# Market feed throughput test
./src/market_feed_test

# Live trading simulation
./src/live_trading
```

## Tests

```bash
ctest --output-on-failure
```

## Benchmarks

From the `build/` directory:

```bash
./benchmarks/signal_engine_benchmark
./benchmarks/aos_vs_soa_benchmark
./benchmarks/pipeline_benchmark
```

## Architecture

The engine supports three operating modes that share common infrastructure:

| Mode | Purpose | Data Source | Concurrency | Use Case |
|------|---------|-------------|-------------|----------|
| **Market Feed** | Live simulation | Producer thread generates ticks | Multi-threaded (producer/consumer/readers) | Throughput testing, live trading prototype |
| **Live Trading** | Real-time signals | Market feed + signal loop | Multi-threaded (feed + trading loop) | Latency benchmarking, live system prototype |
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

### Live Trading Loop (`main_live.cpp`)

The live trading simulation combines the market feed infrastructure with real-time signal computation in a high-frequency trading loop:

```
+------------------+                              +------------------+
|   Market Feed    |                              |  Trading Loop    |
|  (Background)    |                              |   (Main Thread)  |
+------------------+                              +------------------+
        |                                                  |
        v                                                  |
+-------------------+     seqlock snapshot      +----------+---------+
|  UniverseStore    |<--------------------------|   SignalEngine     |
|   (SoA Data)      |                           |  calculate_zscores |
+-------------------+                           +--------------------+
                                                           |
                                                           v
                                                +--------------------+
                                                |   ZScoreStrategy   |
                                                |  BUY/SELL/HOLD     |
                                                +--------------------+
                                                           |
                                                           v
                                                +--------------------+
                                                |   LiveMetrics      |
                                                |  latency, signals  |
                                                +--------------------+
```

**Key characteristics:**

- **Fixed-rate signal generation**: Configurable interval (default 100μs = 10kHz)
- **Non-blocking reads**: Signal computation reads from `UniverseStore` via seqlock without blocking the market data consumer
- **Real-time metrics**: Tracks per-cycle latency (mean, p50, p99, p99.9) and signal distribution
- **Pre-allocated buffers**: All memory allocated upfront to avoid allocation jitter during the hot loop

**Sample output:**
```
Signal Computation Latency:
  Mean:        1329.55 μs
  p50:          495.58 μs
  p99:         8870.49 μs
  p99.9:      12751.80 μs

Signal Distribution:
  Buy Signals:  1.37%
  Sell Signals: 3.15%
  Hold Signals: 95.48%
```

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
- **Live Trading**: Market Feed (background) + fixed-rate signal loop reading via seqlock -> strategy -> metrics
- **Backtester**: Synchronous loop generates prices directly into aligned buffers, calls signals inline
