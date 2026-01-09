#include "backtest/backtester.hpp"
#include <iostream>
#include <iomanip>

int main() {
    constexpr size_t NUM_ASSETS = 1024;
    constexpr size_t NUM_PERIODS = 1000;

    Backtester bt(NUM_ASSETS, NUM_PERIODS);
    StrategyMetrics metrics = bt.run();

    std::cout << "Backtest Results (" << NUM_ASSETS << " assets, " << NUM_PERIODS << " periods):" << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Total PnL:    " << metrics.total_pnl << "\n";
    std::cout << "Sharpe Ratio: " << metrics.sharpe_ratio << "\n";

    return 0;
}