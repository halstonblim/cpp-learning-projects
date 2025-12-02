#include <benchmark/benchmark.h>
#include "core/universe_store.hpp"
#include <random>

// -------------------------------------------------------
// 1. AoS Definition (The "Naive" Approach)
// -------------------------------------------------------
struct Tick {
    uint32_t asset_id;
    float price;
    float volume;
    float bid;
    float ask;
    // Total size: 20 bytes. 
    // Compilers will likely pad this to 24 or 32 bytes for alignment.
};

// -------------------------------------------------------
// 2. Helper to Generate Data
// -------------------------------------------------------
static constexpr size_t NUM_ASSETS = 1'000'000; // 1 Million assets to bust L3 Cache

std::vector<Tick> generate_aos_data() {
    std::vector<Tick> data(NUM_ASSETS);
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dist(10.0f, 1000.0f);

    for (size_t i = 0; i < NUM_ASSETS; ++i) {
        data[i] = {
            static_cast<uint32_t>(i), 
            dist(gen), // Price
            100.0f,    // Vol
            dist(gen), // Bid
            dist(gen)  // Ask
        };
    }
    return data;
}

// -------------------------------------------------------
// 3. Benchmark Functions
// -------------------------------------------------------

// Scenario: Calculate the sum of all prices (e.g., for an index or average).

void BM_AoS_Sum(benchmark::State& state) {
    // Setup
    auto data = generate_aos_data();
    
    for (auto _ : state) {
        float sum = 0.0f;
        // CRITICAL: We iterate through structs, hopping over unused fields (vol, bid, ask)
        // to get to the next 'price'. This wastes cache lines.
        for (const auto& tick : data) {
            sum += tick.price;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * NUM_ASSETS);
    state.SetBytesProcessed(state.iterations() * NUM_ASSETS * sizeof(float)); // We effectively read 1 float per item
}


void BM_SoA_Sum(benchmark::State& state) {
    // Setup
    UniverseStore store(NUM_ASSETS);
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dist(10.0f, 1000.0f);
    
    // Populate Store using our Hot Path function
    for (size_t i = 0; i < NUM_ASSETS; ++i) {
        store.update_tick({
            static_cast<uint32_t>(i),
            dist(gen), // Price
            100.0f,
            dist(gen),
            dist(gen)
        });
    }    

    const float* prices = store.get_prices();

    for (auto _ : state) {
        float sum = 0.0f;
        // CRITICAL: We iterate a contiguous array of floats.
        // Every cache line loaded contains 16 useful prices (64 bytes / 4 bytes).
        // Zero wasted bandwidth.
        for (size_t i = 0; i < NUM_ASSETS; ++i) {
            sum += prices[i];
        }
        benchmark::DoNotOptimize(sum);
    }
        state.SetItemsProcessed(state.iterations() * NUM_ASSETS);
        state.SetBytesProcessed(state.iterations() * NUM_ASSETS * sizeof(float));
}

BENCHMARK(BM_AoS_Sum);
BENCHMARK(BM_SoA_Sum);

BENCHMARK_MAIN();