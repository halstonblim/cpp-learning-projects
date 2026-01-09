#include <benchmark/benchmark.h>
#include "signals/momentum_signal.hpp"
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
    std::vector<float, AlignedAllocator<float, 32>> volumes;
    std::vector<float, AlignedAllocator<float, 32>> output_zscore;
    std::vector<float, AlignedAllocator<float, 32>> output_momentum;

    void SetUp(const ::benchmark::State&) {
        prices.resize(NUM_ASSETS);
        volumes.resize(NUM_ASSETS);
        output_zscore.resize(NUM_ASSETS);
        output_momentum.resize(NUM_ASSETS);

        std::mt19937 gen(42);
        std::uniform_real_distribution<float> dist(10.0f, 100.0f);
        for(auto& p : prices) p = dist(gen);
        for(auto& p : volumes) p = dist(gen);

    }
};

// 2. Define the Pipeline Type
using BM_Pipeline = SignalPipeline<ZScoreSignal,MomentumSignal>;

// 3. Benchmark
BENCHMARK_DEFINE_F(PipelineFixture, BM_Pipeline)(benchmark::State& state) {
    MarketSnapshot snap{prices.data(), volumes.data(), NUM_ASSETS};
    float* outputs[] = { output_zscore.data(), output_momentum.data() };

    for (auto _ : state) {
        BM_Pipeline::execute(snap, outputs);
        benchmark::DoNotOptimize(output_zscore.data());
        benchmark::DoNotOptimize(output_momentum.data());

    }
}
BENCHMARK_REGISTER_F(PipelineFixture, BM_Pipeline)->Iterations(10000);

// 4. Compare against Raw Direct Call (Baseline)
BENCHMARK_DEFINE_F(PipelineFixture, BM_Direct)(benchmark::State& state) {
    MarketSnapshot snap{prices.data(), volumes.data(), NUM_ASSETS};
    
    for (auto _ : state) {
        ZScoreSignal::calculate(snap, output_zscore.data());
        MomentumSignal::calculate(snap, output_momentum.data());
        benchmark::DoNotOptimize(output_zscore.data());
        benchmark::DoNotOptimize(output_momentum.data());    
    }
}
BENCHMARK_REGISTER_F(PipelineFixture, BM_Direct)->Iterations(10000);

BENCHMARK_MAIN();