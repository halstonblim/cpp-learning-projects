#pragma once

#include <cstdint>
#include <iostream>

enum class Side : uint8_t {
    BUY = 0,
    SELL = 1,
    UNKNOWN = 2
};

// PADDING:
// We pad to 32 bytes (power of 2) to ensure that if the array 
// is aligned, no single Tick will ever straddle across two 
// cache lines (avoiding double-fetch penalties).
//
// NOTE: This does NOT prevent false sharing between neighbors. 
// Two ticks will share one 64-byte cache line.

struct Tick {
    double price; // 8 bytes
    double quantity; // 8 bytes
    uint64_t timestamp; //8 bytes
    Side side; // 1 byte
    uint8_t _padding[7]; // 7 bytes
};

static_assert(sizeof(Tick) == 32, "Tick struct size is not 32 bytes");

inline std::ostream& operator<<(std::ostream& os, const Tick& t) {
    os << "[Time: " << t.timestamp 
       << " | Side: " << (t.side == Side::BUY ? "BID" : "ASK") 
       << " | Px: " << t.price 
       << " | Qty: " << t.quantity << "]";
    return os;
}