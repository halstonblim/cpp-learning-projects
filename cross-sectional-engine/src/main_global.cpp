#include "backtest/backtester_global.hpp"

#include <iostream>
#include <iomanip>

int main() {
    constexpr size_t NUM_ASSETS = 2048;
    constexpr size_t NUM_PERIODS = 100000;

    BacktesterGlobal bt(NUM_ASSETS, NUM_PERIODS);
    StrategyMetrics metrics = bt.run();

    std::cout << "Global Backtest Results (" << NUM_ASSETS << " assets, " << NUM_PERIODS << " periods):" << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Total PnL:    " << metrics.total_pnl << "\n";
    std::cout << "Sharpe Ratio: " << metrics.sharpe_ratio << "\n";

    return 0;
}
