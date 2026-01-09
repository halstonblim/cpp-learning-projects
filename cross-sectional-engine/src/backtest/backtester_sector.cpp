#include "backtest/backtester_sector.hpp"
#include "signals/sector_signal.hpp"
#include "strategy/pnl_calculator.hpp"
#include "strategy/zscore_strategy.hpp"
#include <random>

BacktesterSector::BacktesterSector(size_t num_assets, size_t num_periods, size_t num_sectors) 
: num_periods_{num_periods}, num_assets_{num_assets}, num_sectors_{num_sectors}
{
    prev_prices_.resize(num_assets);
    curr_prices_.resize(num_assets);
    returns_.resize(num_assets);
    zscores_.resize(num_assets);
    signals_.resize(num_assets);
    pnl_series_.resize(num_periods);
    sorted_prices_.resize(num_assets);
    sorted_zscores_.resize(num_assets);

    std::mt19937 gen(42);
    std::uniform_int_distribution<uint32_t> sector_rng(0, num_sectors-1);
    std::vector<uint32_t> sector_picks(num_assets);
    for (uint32_t& asset_sector : sector_picks) asset_sector = sector_rng(gen);
    sector_index_ = std::make_unique<SectorIndex>(num_assets, num_sectors, sector_picks);
}

void BacktesterSector::sort_to_sector_order(const float* src, float* dst) {
    for (size_t i = 0; i < num_assets_; ++i) {
        dst[sector_index_->sorted_index(i)] = src[i];
    }
}

void BacktesterSector::unsort_from_sector_order(const float* src, float* dst) {
    for (size_t i = 0; i < num_assets_; ++i) {
        dst[i] = src[sector_index_->sorted_index(i)];
    }
}

StrategyMetrics BacktesterSector::run() {

    std::mt19937 gen(42);
    std::uniform_real_distribution<float> initial(10.0f, 100.0f);
    std::uniform_real_distribution<float> noise(-0.05f, 0.05f);

    for(auto& p : prev_prices_) p = initial(gen);
    for (size_t t=0; t<num_periods_; ++t) {

        for (size_t i=0; i<num_assets_; ++i) {
            curr_prices_[i] = prev_prices_[i] * (1.0f + noise(gen));
        }

        sort_to_sector_order(prev_prices_.data(), sorted_prices_.data());
        SectorNeutralSignal::calculate(sorted_prices_.data(), num_assets_, *sector_index_, sorted_zscores_.data());
        unsort_from_sector_order(sorted_zscores_.data(), zscores_.data());

        ZScoreStrategy::calculate({zscores_.data()}, signals_.data(), num_assets_);
        PnLCalculator::calculate_returns(prev_prices_.data(), curr_prices_.data(), returns_.data(), num_assets_);
        pnl_series_[t] = PnLCalculator::calculate_pnl(signals_.data(), returns_.data(), num_assets_);
        std::swap(prev_prices_, curr_prices_);
    }

    return calculate_metrics(pnl_series_.data(), num_periods_, 0.0f);
}
