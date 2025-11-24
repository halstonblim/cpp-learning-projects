#include <iostream>
#include <thread>
#include <atomic>

#include "types.hpp"
#include "lock_based_queue.hpp"
#include "market_sim.hpp"
#include "signal_engine.hpp"

int main() {
    // 1. Create the Shared Resource (The Queue)
    ThreadSafeQueue<Tick> book_queue;

    // 2. Instantiate the Worker Classes
    // They now have no dependency on the queue
    MarketSimulator sim;
    SignalEngine engine;

    std::atomic<bool> running{true};

    std::cout << "Starting Trading Engine (Lock-Based)..." << std::endl;

    // 3. Launch Threads
    std::jthread producer_thread([&]() {
        while (running) {
            Tick t = sim.next_tick();
            book_queue.push(t);
            // Simulate network/market delay
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    std::jthread consumer_thread([&]() {
        Tick t;
        while (running) {
            // Blocking pop - efficient for lock-based
            book_queue.wait_and_pop(t);
            engine.process_tick(t);
        }
    });

    // 4. Let it run for a while
    std::cout << "Engine running. Press Enter to stop..." << std::endl;
    std::cin.get(); // Blocks main thread until you hit Enter

    // 5. Cleanup
    std::cout << "Stopping..." << std::endl;
    running = false;
    
    // Note: consumer might be stuck in wait_and_pop. 
    // In a real system we'd push a poison pill or use a timeout.
    // For this simple example, we'll just detach or let the program exit (jthread will try to join).
    // To avoid hang on join, we can push a dummy tick to wake up consumer.
    book_queue.push(Tick{}); 

    std::cout << "Engine stopped cleanly." << std::endl;
    
    return 0;
}
