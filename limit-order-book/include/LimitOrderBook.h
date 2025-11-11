#pragma once

#include "Order.h"
#include <map>
#include <functional>

class LimitOrderBook {
public:
    LimitOrderBook() = default;
    void add_order(const Order& order);
    void print_book() const;

private:
    using Bids = std::map<int, int, std::greater<int>>;
    using Asks = std::map<int, int, std::less<int>>;

    Bids bids;
    Asks asks;
};