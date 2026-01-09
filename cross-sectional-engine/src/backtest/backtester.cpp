#include "backtest/backtester.hpp"
#include "strategy/pnl_calculator.hpp"
#include "strategy/zscore_strategy.hpp"
#include "signals/zscore_signal.hpp"
#include <random>

Backtester::Backtester(size_t num_assets, size_t num_periods) : num_periods_{num_periods}, num_assets_{num_assets} {
    prev_prices_.resize(num_assets);
    curr_prices_.resize(num_assets);
    returns_.resize(num_assets);
    zscores_.resize(num_assets);
    signals_.resize(num_assets);
    pnl_series_.resize(num_periods);
}

StrategyMetrics Backtester::run() {

    std::mt19937 gen(42);
    std::uniform_real_distribution<float> initial(10.0f, 100.0f);
    std::uniform_real_distribution<float> noise(-0.05f, 0.05f);

    for(auto& p : prev_prices_) p = initial(gen);
    for (size_t t=0; t<num_periods_; ++t) {

        for (size_t i=0; i<num_assets_; ++i) {
            curr_prices_[i] = prev_prices_[i] * (1.0f + noise(gen));
        }

        MarketSnapshot snap{prev_prices_.data(), nullptr, num_assets_};
        ZScoreSignal::calculate(snap, zscores_.data());
        ZScoreStrategy::calculate({zscores_.data()}, signals_.data(), num_assets_);
        PnLCalculator::calculate_returns(prev_prices_.data(), curr_prices_.data(), returns_.data(), num_assets_);
        pnl_series_[t] = PnLCalculator::calculate_pnl(signals_.data(), returns_.data(), num_assets_);
        std::swap(prev_prices_, curr_prices_);
    }

    return calculate_metrics(pnl_series_.data(), num_periods_, 0.0f);
}
