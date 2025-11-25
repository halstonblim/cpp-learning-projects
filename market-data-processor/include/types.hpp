#pragma once

#include <cstdint>
#include <iostream>

enum class Side : uint8_t {
    BUY = 0,
    SELL = 1,
    UNKNOWN = 2
};

// Padded to 32 bytes to prevent cache line straddling.
// Does not prevent false sharing between adjacent ticks in arrays.
struct Tick {
    double price;           // 8 bytes
    double quantity;        // 8 bytes
    uint64_t timestamp;     // 8 bytes (nanoseconds since epoch)
    Side side;              // 1 byte
    uint8_t _padding[7];    // 7 bytes padding to 32-byte alignment
};

static_assert(sizeof(Tick) == 32, "Tick struct size is not 32 bytes");

inline std::ostream& operator<<(std::ostream& os, const Tick& t) {
    os << "[Time: " << t.timestamp
       << " | Side: " << (t.side == Side::BUY ? "BID" : "ASK")
       << " | Px: " << t.price
       << " | Qty: " << t.quantity << "]";
    return os;
}
