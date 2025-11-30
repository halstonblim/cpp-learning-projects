#include <benchmark/benchmark.h>
#include "time_series.hpp"

void FillTimeSeries(TimeSeries& ts, size_t n) {
    ts.clear();
    for (size_t i = 0; i < n; ++i) {
        ts.add_tick(static_cast<double>(i) + 0.5);
    }
}


void BenchmarkScalarMean(benchmark::State& state) {
    size_t n = state.range(0);
    TimeSeries ts(n);
    FillTimeSeries(ts, n);

    double result;
    for (auto _ : state) {
        result = ts.get_mean();
        benchmark::DoNotOptimize(result);
    }
}

void BenchmarkAVXMean(benchmark::State& state) {
    size_t n = state.range(0);
    TimeSeries ts(n);
    FillTimeSeries(ts, n);

    double result;
    for (auto _ : state) {
        result = ts.get_mean_simd();
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BenchmarkScalarMean)->Range(8192, 8<<20);
BENCHMARK(BenchmarkAVXMean)->Range(8192, 8<<20);
BENCHMARK_MAIN();