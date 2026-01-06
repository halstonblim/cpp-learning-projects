#include <benchmark/benchmark.h>
#include "signals/pipeline.hpp"
#include "signals/zscore_signal.hpp"
#include "core/aligned_allocator.hpp"
#include <vector>
#include <random>

// 1. Setup Data
constexpr size_t NUM_ASSETS = 1'000'000;

class PipelineFixture : public benchmark::Fixture {
public:
    std::vector<float, AlignedAllocator<float, 32>> prices;
    std::vector<float, AlignedAllocator<float, 32>> volumes; // Unused but part of snapshot
    std::vector<float, AlignedAllocator<float, 32>> output_buffer;
    
    void SetUp(const ::benchmark::State&) {
        prices.resize(NUM_ASSETS);
        volumes.resize(NUM_ASSETS);
        output_buffer.resize(NUM_ASSETS);

        std::mt19937 gen(42);
        std::uniform_real_distribution<float> dist(10.0f, 100.0f);
        for(auto& p : prices) p = dist(gen);
    }
};

// 2. Define the Pipeline Type
using MyPipeline = SignalPipeline<ZScoreSignal>;

// 3. Benchmark
BENCHMARK_F(PipelineFixture, BM_Pipeline_ZScore)(benchmark::State& state) {
    MarketSnapshot snap{prices.data(), volumes.data(), NUM_ASSETS};
    float* outputs[] = { output_buffer.data() }; // Array of output pointers

    for (auto _ : state) {
        MyPipeline::execute(snap, outputs);
        benchmark::DoNotOptimize(output_buffer.data());
    }
}

// 4. Compare against Raw Direct Call (Baseline)
BENCHMARK_F(PipelineFixture, BM_Direct_ZScore)(benchmark::State& state) {
    MarketSnapshot snap{prices.data(), volumes.data(), NUM_ASSETS};
    
    for (auto _ : state) {
        ZScoreSignal::calculate(snap, output_buffer.data());
        benchmark::DoNotOptimize(output_buffer.data());
    }
}

BENCHMARK_MAIN();