#include "backtest/backtester_sector.hpp"

#include <iostream>
#include <iomanip>

int main() {
    constexpr size_t NUM_ASSETS = 2048;
    constexpr size_t NUM_PERIODS = 100000;
    constexpr size_t NUM_SECTORS = 10;

    BacktesterSector bts(NUM_ASSETS, NUM_PERIODS, NUM_SECTORS);
    StrategyMetrics metrics = bts.run();

    std::cout << "Sector Backtest Results (" << NUM_ASSETS << " assets, " << NUM_PERIODS << " periods):" << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Total PnL:    " << metrics.total_pnl << "\n";
    std::cout << "Sharpe Ratio: " << metrics.sharpe_ratio << "\n";

    return 0;
}
