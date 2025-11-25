#pragma once
#include <vector>
#include <atomic>
#include <optional>

// Single-producer, single-consumer lock-free ring buffer queue.
template<typename T>
class LockFreeQueue {
public:
    // Allocates size+1 slots (one slot reserved to distinguish full vs empty)
    explicit LockFreeQueue(size_t size)
        : buffer_(size + 1), head_(0), tail_(0) {}

    // Returns false if queue is full
    bool push(const T& item) {
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t current_tail = tail_.load(std::memory_order_acquire);
        size_t next_head = (current_head + 1) % buffer_.size();

        if (next_head == current_tail) {
            return false;  // Full
        }

        // Safe: consumer won't read this slot until head is published
        buffer_[current_head] = item;

        // Release: ensures write completes before consumer sees new head
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    // Returns nullopt if queue is empty
    [[nodiscard]] std::optional<T> pop() {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t current_head = head_.load(std::memory_order_acquire);

        if (current_head == current_tail) {
            return std::nullopt;  // Empty
        }

        T item = buffer_[current_tail];
        size_t next_tail = (current_tail + 1) % buffer_.size();

        // Release: signals producer that slot is now free
        tail_.store(next_tail, std::memory_order_release);
        return item;
    }

private:
    std::vector<T> buffer_;

    // Cache-line aligned to prevent false sharing between producer/consumer
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
};
