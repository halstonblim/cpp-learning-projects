#pragma once

#include <iostream>

enum class Side {
    BUY, SELL
};

struct Order {
    Side side;
    int quantity;
    int price;
};

inline std::ostream& operator<<(std::ostream& os, Side side) {
    os << (side == Side::BUY ? "BUY" : "SELL");
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Order& order) {
    os << order.side << " " << order.quantity << " at " << order.price;
    return os;
}