// Single-threaded burst push/pop benchmark comparing queue implementations.

#include <benchmark/benchmark.h>
#include "lock_based_queue.hpp"
#include "lock_free_queue.hpp"
#include "types.hpp"

constexpr int BURST_SIZE = 100;  // Fits in cache, under ring buffer capacity

namespace {

void BM_LockBasedQueue(benchmark::State& state) {
    ThreadSafeQueue<Tick> queue;
    Tick dummy_tick{};

    for (auto _ : state) {
        for (int i = 0; i < BURST_SIZE; ++i) {
            queue.push(dummy_tick);
        }

        for (int i = 0; i < BURST_SIZE; ++i) {
            Tick t;
            bool success = queue.try_pop(t);
            benchmark::DoNotOptimize(success);
            benchmark::DoNotOptimize(t);
        }
    }
}

void BM_LockFreeQueue(benchmark::State& state) {
    LockFreeQueue<Tick> queue(1024);
    Tick dummy_tick{};

    for (auto _ : state) {
        for (int i = 0; i < BURST_SIZE; ++i) {
            queue.push(dummy_tick);
        }

        for (int i = 0; i < BURST_SIZE; ++i) {
            auto res = queue.pop();
            benchmark::DoNotOptimize(res);
        }
    }
}

}  // namespace

BENCHMARK(BM_LockBasedQueue);
BENCHMARK(BM_LockFreeQueue);

BENCHMARK_MAIN();
