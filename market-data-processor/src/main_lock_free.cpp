#include <iostream>
#include <thread>
#include <atomic>
#include <optional>

#include "types.hpp"
#include "lock_free_queue.hpp"
#include "market_sim.hpp"
#include "signal_engine.hpp"

int main() {
    // 1. Create the Lock-Free Queue
    LockFreeQueue<Tick> queue(1024); 

    // 2. Instantiate Workers
    MarketSimulator sim;
    SignalEngine engine;

    std::atomic<bool> running{true};
    
    std::cout << "Starting Trading Engine (Lock-Free)..." << std::endl;

    // 3. Launch Producer (Market Simulator)
    std::jthread producer([&]() {
        while (running) {
            Tick t = sim.next_tick();
            
            // SPIN LOOP: If full, keep trying
            while (!queue.push(t) && running) {
                // Busy wait or yield
                std::this_thread::yield(); 
            }
            
            // Simulate market interval
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    // 4. Launch Consumer (Signal Engine)
    std::jthread consumer([&]() {
        while (running) {
            auto tick_opt = queue.pop();
            if (tick_opt) {
                engine.process_tick(*tick_opt);
            } else {
                // SPIN: If empty, just try again immediately (Busy Wait)
                // In low latency, we don't sleep.
            }
        }
    });

    // 5. Run for 5 seconds then stop (or wait for user input)
    std::cout << "Engine running. Press Enter to stop..." << std::endl;
    std::cin.get();

    running = false;
    std::cout << "Stopping..." << std::endl;
    
    return 0;
}
