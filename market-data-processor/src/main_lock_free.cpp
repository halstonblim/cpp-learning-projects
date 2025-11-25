#include <iostream>
#include <thread>
#include <atomic>
#include <optional>

#include "types.hpp"
#include "lock_free_queue.hpp"
#include "market_sim.hpp"
#include "signal_engine.hpp"

int main() {
    LockFreeQueue<Tick> queue(1024);
    MarketSimulator sim;
    SignalEngine engine;

    std::atomic<bool> running{true};

    std::cout << "Starting Trading Engine (Lock-Free)..." << std::endl;

    // Producer thread (spin-retry on full queue)
    std::jthread producer([&]() {
        while (running) {
            Tick t = sim.next_tick();

            while (!queue.push(t) && running) {
                std::this_thread::yield();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    // Consumer thread (busy-wait polling)
    std::jthread consumer([&]() {
        while (running) {
            auto tick_opt = queue.pop();
            if (tick_opt) {
                engine.process_tick(*tick_opt);
            }
            // Empty queue: spin without sleeping for low latency
        }
    });

    std::cout << "Engine running. Press Enter to stop..." << std::endl;
    std::cin.get();

    running = false;
    std::cout << "Stopping..." << std::endl;

    return 0;
}
