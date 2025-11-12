#include "LimitOrderBook.h"
#include <iostream>
#include <algorithm> // For std::min

void LimitOrderBook::add_order(Order order) {
    if (order.side == Side::BUY) {
        while (order.quantity > 0 && !asks.empty() && order.price >= asks.begin()->first) {
            auto best_ask_it =  asks.begin();
            int& best_ask_vol = best_ask_it->second;

            int traded_vol = std::min(best_ask_vol, order.quantity);

            order.quantity -= traded_vol;
            best_ask_vol -= traded_vol;

            if (best_ask_vol == 0) asks.erase(best_ask_it);        
        }        
        if (order.quantity > 0) {
            bids[order.price] += order.quantity;
        }
    }
    else {
        while (order.quantity > 0 && !bids.empty() && order.price <= bids.begin()->first) {
            auto best_bid_it =  bids.begin();
            int& best_bid_vol = best_bid_it->second;

            int traded_vol = std::min(best_bid_vol, order.quantity);

            order.quantity -= traded_vol;
            best_bid_vol -= traded_vol;

            if (best_bid_vol == 0) bids.erase(best_bid_it);        
        }        
        if (order.quantity > 0) {
            asks[order.price] += order.quantity;
        }
    }
}

void LimitOrderBook::print_book() const {
    std::cout << "--- ORDER BOOK ---" << std::endl;

    std::cout << " ASKS (Price: Qty)" << std::endl;
    for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
        std::cout << "(" << it->first << ": " << it->second << ")" << std::endl;
    }

    std::cout << " BIDS (Price: Qty)" << std::endl;
    for (const auto& [price, quantity] : bids) {
        std::cout << "(" << price << ": " << quantity << ")" << std::endl;
    }

    std::cout << "---------------------" << std::endl;
    
}