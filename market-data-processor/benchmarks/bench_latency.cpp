#include <benchmark/benchmark.h>
#include "lock_based_queue.hpp"
#include "lock_free_queue.hpp"
#include "types.hpp" // for Tick

// Define a small "burst" size that fits easily in cache
// and is smaller than the Ring Buffer size (default 1024 in our tests)
constexpr int BURST_SIZE = 100;

namespace {

// --- Benchmark 1: Lock-Based Queue (Mutex) ---
void BM_LockBasedQueue(benchmark::State& state) {
    ThreadSafeQueue<Tick> queue;
    Tick dummy_tick{}; 

    for (auto _ : state) {
        // Phase 1: Burst Push
        for (int i = 0; i < BURST_SIZE; ++i) {
            queue.push(dummy_tick);
        }

        // Phase 2: Burst Pop
        for (int i = 0; i < BURST_SIZE; ++i) {
            Tick t;
            // We use try_pop to avoid blocking indefinitely if logic fails
            queue.try_pop(t); 
        }
    }
}

// --- Benchmark 2: Lock-Free Queue (Atomic) ---
void BM_LockFreeQueue(benchmark::State& state) {
    // Initialize with enough space for the burst + padding
    LockFreeQueue<Tick> queue(1024);
    Tick dummy_tick{};

    for (auto _ : state) {
        // Phase 1: Burst Push
        for (int i = 0; i < BURST_SIZE; ++i) {
            // In a real app, we'd handle the "false" return (full)
            // Here we assume it fits for the benchmark
            queue.push(dummy_tick);
        }

        // Phase 2: Burst Pop
        for (int i = 0; i < BURST_SIZE; ++i) {
            // In single-thread bench, this should never fail if logic is correct
            auto res = queue.pop();
            benchmark::DoNotOptimize(res); // Prevent compiler from optimizing away the pop
        }
    }
}

} // anonymous namespace

BENCHMARK(BM_LockBasedQueue);
BENCHMARK(BM_LockFreeQueue);

BENCHMARK_MAIN();