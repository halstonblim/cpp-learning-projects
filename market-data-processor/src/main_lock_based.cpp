#include <iostream>
#include <thread>
#include <atomic>

#include "types.hpp"
#include "lock_based_queue.hpp"
#include "market_sim.hpp"
#include "signal_engine.hpp"

int main() {
    ThreadSafeQueue<Tick> book_queue;
    MarketSimulator sim;
    SignalEngine engine;

    std::atomic<bool> running{true};

    std::cout << "Starting Trading Engine (Lock-Based)..." << std::endl;

    // Producer thread
    std::jthread producer_thread([&]() {
        while (running) {
            Tick t = sim.next_tick();
            book_queue.push(t);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    // Consumer thread (blocking pop)
    std::jthread consumer_thread([&]() {
        Tick t;
        while (running) {
            book_queue.wait_and_pop(t);
            engine.process_tick(t);
        }
    });

    std::cout << "Engine running. Press Enter to stop..." << std::endl;
    std::cin.get();

    std::cout << "Stopping..." << std::endl;
    running = false;

    // Push dummy tick to unblock consumer waiting on empty queue
    book_queue.push(Tick{});

    std::cout << "Engine stopped cleanly." << std::endl;

    return 0;
}
