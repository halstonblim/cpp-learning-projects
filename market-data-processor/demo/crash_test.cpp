// Demonstrate segmentation fault without thread locking
// g++ -std=c++20 -pthread crash_test.cpp -o crash_test

#include <iostream>
#include <queue>
#include <thread>

struct Tick {
    double price;
    double quantity;
};

int main() {
    std::queue<Tick> unsafe_queue;
    bool keep_running = true;

    std::jthread productor([&]() {
        for (int i = 0; i < 100000; ++i) {
            unsafe_queue.push({100.0, 10.0});
        }
        keep_running = false;
    });

    std::jthread consumer([&]() {
        while (keep_running || !unsafe_queue.empty()) {
            if (!unsafe_queue.empty()) {
                unsafe_queue.pop();
            }
        }
    });
    std::cout << "Race started... waiting for crash." << std::endl;
    return 0;
}