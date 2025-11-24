#include <iostream>
#include <thread>
#include <vector>

#include "types.hpp"
#include "lock_based_queue.hpp"
#include "market_sim.hpp"
#include "signal_engine.hpp"

int main() {
    // 1. Create the Shared Resource (The Queue)
    ThreadSafeQueue<Tick> book_queue;

    // 2. Instantiate the Worker Classes
    // We pass the queue by reference so they share it.
    MarketSimulator producer(book_queue);
    SignalEngine consumer(book_queue);

    std::cout << "Starting Trading Engine..." << std::endl;

    // 3. Launch Threads
    // std::jthread is a C++20 feature that automatically joins when it goes out of scope.
    // It's safer than std::thread because you can't forget .join()
    std::jthread producer_thread([&producer]() {
        producer.run(); 
    });

    std::jthread consumer_thread([&consumer]() {
        consumer.run(); 
    });

    // 4. Let it run for a while
    std::cout << "Engine running. Press Enter to stop..." << std::endl;
    std::cin.get(); // Blocks main thread until you hit Enter

    // 5. Cleanup
    std::cout << "Stopping..." << std::endl;
    producer.stop();
    consumer.stop();

    // Threads will join automatically here due to jthread destructor
    std::cout << "Engine stopped cleanly." << std::endl;
    
    return 0;
}