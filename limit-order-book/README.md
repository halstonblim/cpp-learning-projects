# Limit Order Book

A C++ implementation of a limit order book.

## Summary

This project implements a limit order book that matches buy and sell orders based on price priority. Orders automatically match against the best available price on the opposite side when prices cross. Orders at the same price level are aggregated, and unmatched orders are added to the book as resting orders.

## Building

To build:

1. Create a build directory: `mkdir build && cd build`
2. Configure the build system: `cmake ..`
3. Build: `cmake --build .`

The executable `lob_driver` will be created in the build directory.

## Running

The driver program (`lob_driver`) demonstrates the order book functionality by adding initial resting orders (asks and bids) and processing subsequent orderes by either matching against existing liquidity or adding to the book.

Run the executable to see the order book state after each operation.