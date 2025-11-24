#pragma once
#include <vector>
#include <atomic>
#include <optional> // C++17 feature, very useful here

template<typename T>
class LockFreeQueue {
public:
    explicit LockFreeQueue(size_t size) 
        : buffer_(size + 1), head_(0), tail_(0) {}

    bool push(const T& item) {
        // 1. Load the current indices
        // relaxing order is fine for loading; we only need strictness when publishing
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t current_tail = tail_.load(std::memory_order_acquire);

        // 2. Calculate next head position
        size_t next_head = (current_head + 1) % buffer_.size();

        // 3. Check if full
        if (next_head == current_tail) {
            return false; // Queue is full, producer must retry or drop
        }

        // 4. Write the data (Non-atomic write is safe because consumer isn't looking here yet)
        buffer_[current_head] = item;

        // 5. Publish the change
        // "release" ensures the consumer doesn't see the new head 
        // until the data write above is visibly complete.
        head_.store(next_head, std::memory_order_release);
        
        return true;
    }

    std::optional<T> pop() {
        // 1. Load current indices
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t current_head = head_.load(std::memory_order_acquire);

        // 2. Check if empty
        if (current_head == current_tail) {
            return std::nullopt; // Queue is empty
        }

        // 3. Read the data
        T item = buffer_[current_tail];

        // 4. Calculate next tail
        size_t next_tail = (current_tail + 1) % buffer_.size();

        // 5. Publish the change
        // "release" tells the producer a slot is now free
        tail_.store(next_tail, std::memory_order_release);

        return item;
    }

private:
    std::vector<T> buffer_;

    // Alignment to eliminate false sharing
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
};