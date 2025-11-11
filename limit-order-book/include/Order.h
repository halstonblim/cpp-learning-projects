#pragma once

#include <iostream>

enum class Side {
    BUY, SELL
};

struct Order {
    int id;
    Side side;
    double price;
    int quantity;
};

inline std::ostream& operator<<(std::ostream& os, Side side) {
    os << (side == Side::BUY ? "BUY" : "SELL");
    return os;
}