# Limit Order Book

A C++ implementation of a limit order book.

## Summary

This project implements a limit order book that matches buy and sell orders based on price-time priority. Orders are automatically matched when prices cross, and unmatched orders are added to the book as resting orders.

## Building

To build:

1. Create a build directory: `mkdir build && cd build`
2. Configure the build system: `cmake ..`
3. Build: `cmake --build .`

The executable `lob_driver` will be created in the build directory.