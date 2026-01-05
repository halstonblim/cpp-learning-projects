#include <benchmark/benchmark.h>
#include "core/universe_store.hpp"
#include "compute/signal_engine.hpp"
#include <random>

namespace {

void fill_random_data(UniverseStore& store) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> price_dist(10.0f, 1000.0f);
    std::uniform_real_distribution<float> vol_dist(1.0f, 1000.0f);

    for (size_t i = 0; i < store.capacity(); ++i) {
        store.update_tick({
            static_cast<uint32_t>(i),
            price_dist(gen),
            vol_dist(gen),
            0.0f, 0.0f // Bid/Ask not needed for this calc
        });
    }
}

void BM_Signal_Scalar(benchmark::State& state) {
    size_t num_assets = state.range(0);
    UniverseStore store(num_assets);
    fill_random_data(store);
    SignalEngine engine(store);
    float total;

    for (auto _ : state) {
        total = engine.calculate_total_notional_scalar();
        benchmark::DoNotOptimize(total);
    }
    state.SetItemsProcessed(state.iterations() * num_assets);
}

void BM_Signal_AVX(benchmark::State& state) {
    size_t num_assets = state.range(0);
    UniverseStore store(num_assets);
    fill_random_data(store);
    SignalEngine engine(store);
    float total;

    for (auto _ : state) {
        total = engine.calculate_total_notional_avx();
        benchmark::DoNotOptimize(total);
    }
    state.SetItemsProcessed(state.iterations() * num_assets);
}

} // namespace

// Run with 1 Million assets (L3 Cache buster size)
BENCHMARK(BM_Signal_Scalar)->Arg(1'000'000);
BENCHMARK(BM_Signal_AVX)->Arg(1'000'000);

BENCHMARK_MAIN();